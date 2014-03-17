//
// Firmware for Ahoy AVR MCU 
// Author: Dean Blackketter, Ahoy
// Copyright 2014, All Rights Reserved
//
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "stdlib.h"
#include "spi.h"

#if 0
// fuse information is not propagated to the hex file, so the correct fuse
// settings are passed via the command line to avrdude, see "upgrade" script
#include <avr/fuse.h>

FUSES = 
  {
    .low = FUSE_CKSEL0 & FUSE_SUT0,
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
  };
#endif

///////////////////////////////////////////////////////////////////////////////
// Button related defines and globals
///////////////////////////////////////////////////////////////////////////////

#define BUTTON_INPUT        PINC
#define BUTTON_PORT         PORTC
#define BUTTON_PORT_DIRECTION DDRC

#define MAIN_BUTTON (1)
#define UP_BUTTON (2)
#define DOWN_BUTTON (4)
#define SETUP_BUTTON (8)
#define BUTTON_MASK (0x0f)

volatile char button_state;                         // debounced and inverted button state: 
                                                    // bit = 1: button pressed 
volatile char button_press;                         // button press detect 

volatile uint8_t colorTicks = 0;  // increases every button test, approx 120Hz.

///////////////////////////////////////////////////////////////////////////////
// LED related defines and globals
///////////////////////////////////////////////////////////////////////////////

#define LED_PORT            PORTD
#define LED_PORT_DIRECTION  DDRD

#define RED_LED (4)
#define GREEN_LED (2)
#define BLUE_LED (1)

// these are also on the LED port
#define CODEC_RESET (0x40)
#define SYS_RESET (0x80)

#define LED_MASK (0x07)


// Default brightnesses at boot time
#define INIT_RED (0)
#define INIT_GREEN (0)
#define INIT_BLUE (0)

// LEDs start off dark
volatile uint8_t red = INIT_RED;
volatile uint8_t green = INIT_GREEN;
volatile uint8_t blue = INIT_BLUE;


//////////////////////////////////////////////////////////////////////////
// Color commands
//////////////////////////////////////////////////////////////////////////

/*

Each SPI byte sent back to host includes the current button status in the low 4 bits

Colors are on a quasi logarithmic curves. 
Time is roughly 43 ticks per second
Slew rate is in color units per tick

Commands:

0x00 - nop
0x83   0xRR 0xGG 0xBB  - set color
0x84   0xRR 0xGG 0xBB 0xSS - fade to color slew rate SS
0x8A   0xR0 0xG0 0xB0 0xS0 0xT0   0xR1 0xG1 0xB1 0xS1 0xT2 - fade fade in and out (slew in Sn, duration Tn)

*/

typedef struct color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t slew; // 0 = immediate
    uint8_t time; // 0 = and stop
} color;

typedef union {
  uint8_t bytes[16];
  color color[2];
} command;

// one active command, one new command
volatile command commands[2];
volatile uint8_t newCommandReady = 0;

// keeping track of which command we are processing
volatile uint8_t nextCommand = 1;
volatile uint8_t activeCommand = 0;

//////////////////////////////////////////////////////////////////////////
// LED PWM interrupt routine
// on 16-bit timer1
//////////////////////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect)
{
  static uint8_t pwm_index = 0;
  pwm_index++;

  uint8_t color = LED_MASK;   // high is off, low is on, mask is all off

  if (pwm_index < red) {
    color -= RED_LED;  // subtract to turn on
  }
  if (pwm_index < green) {
    color -= GREEN_LED;
  }
  if (pwm_index < blue) {
    color -= BLUE_LED;
  }
  
  LED_PORT = color;
}

//////////////////////////////////////////////////////////////////////////
// button interrupt service routine on 8-bit timer0
//////////////////////////////////////////////////////////////////////////
ISR(TIMER0_OVF_vect)
{
  static char ct0, ct1; 
  char i; 

  i = button_state ^ ~BUTTON_INPUT;           // button changed ? 
  ct0 = ~( ct0 & i );                   // reset or count ct0 
  ct1 = ct0 ^ (ct1 & i);                // reset or count ct1 
  i &= ct0 & ct1;                       // count until roll over ? 
  button_state ^= i;                       // then toggle debounced state 
                                        // now debouncing finished 
  button_press |= button_state & i;           // 0->1: button press detect 
  colorTicks++;
}

char get_button_press( char button_mask ) 
{ 
  cli(); 
  button_mask &= button_press;                // read button(s) 
  button_press ^= button_mask;                // clear button(s) 
  sei(); 
  return button_mask; 
} 

//////////////////////////////////////////////////////////////////////////
// SPI Interrupt service routine
//////////////////////////////////////////////////////////////////////////
ISR(SPI_STC_vect)
{
  // keeping track of the incoming command
  static uint8_t commandByteIndex = 0;
  static uint8_t commandBytesRemaining = 0;
 
  uint8_t c = received_from_spi(button_state);
   
   if (commandByteIndex == 0) {
    // receiving byte of command
    switch (c & 0xf0) {
      case (0x80):
        commandByteIndex++; //
        commandBytesRemaining = c & 0x0f;  // how many more bytes left in the command
        break;  
      case 0:
        // nop - reset state
      default:
        // unknown command
        commandByteIndex = 0;
        commandBytesRemaining = 0;
        break;
    }
  } else {
    if (commandBytesRemaining > 0) {
      commands[nextCommand].bytes[commandByteIndex - 1] = c;
      commandByteIndex++;
      commandBytesRemaining--;
    }
    if (commandBytesRemaining == 0) {
      // execute new command
      newCommandReady = 1;
      commandByteIndex = 0;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// color animation based on commands
//////////////////////////////////////////////////////////////////////////

void runColor(void) {

    // run every 3 ticks
    if (colorTicks < 3)
      return;
          
    colorTicks = 0;

  // keeping track of which phase of the color cycle we are in and how many ticks are left 
  static uint8_t cyclePhase = 0;
  static uint8_t cycleTime = 0;
  
  // local log-based versions of red, green and blue
  static uint8_t logred = INIT_RED;
  static uint8_t loggreen = INIT_GREEN;
  static uint8_t logblue = INIT_BLUE;

  if (newCommandReady) {
    newCommandReady = 0;
    
    if (activeCommand == 1) {
       nextCommand = 1;
       activeCommand = 0;
    } else {
      nextCommand = 0;
      activeCommand = 1;
    } 
    
    uint8_t i;
    // clear out the old command
    for (i = 0; i < sizeof(command); i++) {
      commands[nextCommand].bytes[i] = 0;
    }
    cyclePhase = 0;
    cycleTime = 0;
  }
  
  // use the slew to calculate new colors.  
  uint8_t slew = commands[activeCommand].color[cyclePhase].slew;

  // slew of zero is equivalent to infinate slew
  if (slew==0) 
    slew = 0xff;
    
  // calculate new colors given the slew
  uint8_t target = commands[activeCommand].color[cyclePhase].red;
  
  if (target > logred) {
    if ((target - logred) > slew) {
      logred += slew;
    } else {
      logred = target;
    }
  } else { 
    if ((logred - target) > slew) {
      logred -= slew;
    } else {
      logred = target;
    }
  } 

  target = commands[activeCommand].color[cyclePhase].green;
  
  if (target > loggreen) {
    if ((target - loggreen) > slew) {
      loggreen += slew;
    } else {
      loggreen = target;
    }
  } else { 
    if ((loggreen - target) > slew) {
      loggreen -= slew;
    } else {
      loggreen = target;
    }
  } 
  
  target = commands[activeCommand].color[cyclePhase].blue;
  
  if (target > logblue) {
    if ((target - logblue) > slew) {
      logblue += slew;
    } else {
      logblue = target;
    }
  } else { 
    if ((logblue - target) > slew) {
      logblue -= slew;
    } else {
      logblue = target;
    }
  } 

// calculate linear versions
  if (logred < 128) 
    red = logred/2;
  else if (logred < 192)
    red = 64 + (logred - 128); 
  else
    red = 128 + (logred - 192)*2;

  if (loggreen < 128) 
    green = loggreen/2;
  else if (loggreen < 192)
    green = 64 + (loggreen - 128); 
  else
    green = 128 + (loggreen - 192)*2;

  if (logblue < 128) 
    blue = logblue/2;
  else if (logblue < 192)
    blue = 64 + (logblue - 128); 
  else
    blue = 128 + (logblue - 192)*2;
    
  uint8_t time = commands[activeCommand].color[cyclePhase].time;
  
  if (time == 0) {
    // time of 0 means that we don't change phases
  } else {
    cycleTime++;;
    // are we at the end of the phase?
    if (cycleTime >= time) {
      cycleTime = 0;
      cyclePhase = !cyclePhase;
    }
  } 
}
//////////////////////////////////////////////////////////////////////////////
// Local behavior waiting for first command from CPU
//////////////////////////////////////////////////////////////////////////////
void runLocal(void) {

  commands[nextCommand].bytes[0] = 0x80; //r
  commands[nextCommand].bytes[1] = 0x80; //g
  commands[nextCommand].bytes[2] = 0x80; //b
  
  commands[nextCommand].bytes[3] = 0x01; //slew
  commands[nextCommand].bytes[4] = 0x80; //time
  
  commands[nextCommand].bytes[5] = 0x00; //r
  commands[nextCommand].bytes[6] = 0x00; //g
  commands[nextCommand].bytes[7] = 0x00; //b
  
  commands[nextCommand].bytes[8] = 0x01; //slew
  commands[nextCommand].bytes[9] = 0x80; //time
  
  newCommandReady = 1;
  
#ifdef oldanimation
  uint8_t animate = 1;
  uint8_t slowtimer = 0;
  int8_t direction = 1;

   while (!newCommandReady) {
    uint8_t buttons = get_button_press(BUTTON_MASK);   // read the lowest three bits
        
    
    if (buttons & SETUP_BUTTON) {
        animate = !animate;
    }
    
    if (animate) {
      slowtimer++;
      if (slowtimer == 0) {
        red += direction;
        green += direction;
        blue += direction;
        if (red == 0 || red == 0xff) {
          direction = -direction;
        }
      }
    } else {
      if (buttons & MAIN_BUTTON) {
        if (red==0)
        { 
          red = 1; 
         } else if (red == 0xff) {
          red = 0;
       } else {
          red = (red << 1) + 1;
        }
      }
    
      if (buttons & UP_BUTTON) {
        if (green==0)
        { 
          green = 1; 
        } else if (green == 0xff) {
          green = 0;
        } else {
          green = (green << 1) + 1;
        }
      }
    
      if (buttons & DOWN_BUTTON) {
        if (blue==0)
        { 
          blue = 1; 
        } else if (blue == 0xff) {
          blue = 0;
        } else {
          blue = (blue << 1) + 1;
        }
      }   
    }

  }
#endif

}
//////////////////////////////////////////////////////////////////////////
// checkReset
//////////////////////////////////////////////////////////////////////////
void checkReset(void) {

  // only the SETUP and DOWN buttons are pressed
  if ((button_state & BUTTON_MASK) == (SETUP_BUTTON + DOWN_BUTTON)) {
    // flash white/black fast
    commands[nextCommand].bytes[0] = 0xff; //r
    commands[nextCommand].bytes[1] = 0xff; //g
    commands[nextCommand].bytes[2] = 0xff; //b
  
    commands[nextCommand].bytes[3] = 0x00; //slew
    commands[nextCommand].bytes[4] = 0x04; //time
  
    commands[nextCommand].bytes[5] = 0x00; //r
    commands[nextCommand].bytes[6] = 0x00; //g
    commands[nextCommand].bytes[7] = 0x00; //b
  
    commands[nextCommand].bytes[8] = 0x00; //slew
    commands[nextCommand].bytes[9] = 0x04; //time
  
    newCommandReady = 1;

    // pull down reset pins
    LED_PORT_DIRECTION = LED_MASK | SYS_RESET | CODEC_RESET;
    // the LED ISR will now write zeros to the reset pins, as a side effect
    
    // enable watchdog timer for one second
    wdt_enable(WDTO_1S);
    
    // reboot this system after a second
    while (1) {};
  }
}

//////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////
int main(void)
{
  MCUSR = 0;
  wdt_disable();
  
  cli();             // disable global interrupts

//////////////////////////////////////////////////////////////////////////
// Initialize LEDs
//////////////////////////////////////////////////////////////////////////
  LED_PORT_DIRECTION = LED_MASK;           /* make the LED pin an output */
  LED_PORT = 0; 

  // initialize Timer1
  TCCR1A = 0;        // set entire TCCR1A register to 0
  TCCR1B = 0;


  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  
  // turn on CTC (Clear Timer on Compare) mode:
  TCCR1B |= (1 << WGM12);

  // set compare match register to desired timer count:
  OCR1A = 65;  // interrupt every 65 cycles, so 123kHz, (or /256 for 480Hz base frequency in the PWM)
  
  // Full clock speed (no prescaling)
  TCCR1B |= (1 << CS10);

//////////////////////////////////////////////////////////////////////////
// Initialize Buttons
//////////////////////////////////////////////////////////////////////////

  button_state = 0; 
  button_press = 0; 
  
  // initialize timer0
  TCCR0A = (1<<CS02);          // divide by 256
  TIMSK0 = 1<<TOIE0;           // enable timer overflow interrupt (so, 8M/256/256 = 122Hz)

  BUTTON_PORT_DIRECTION = 0;   // all inputs for now
  BUTTON_PORT = BUTTON_MASK;   // enable pullups on the buttons


//////////////////////////////////////////////////////////////////////////
// Initialize SPI
//////////////////////////////////////////////////////////////////////////
  setup_spi(SPI_MODE_0, SPI_MSB, SPI_INTERRUPT, SPI_SLAVE);
  
//////////////////////////////////////////////////////////////////////////
// End Initialization
//////////////////////////////////////////////////////////////////////////
  
  // enable global interrupts:
  sei();
    
//////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////

  runLocal();
  
  for(;;){
    // iterate the color animation
    runColor();
    
    // check to see if the magic button combination is set to reset the host
    checkReset();
    
  }
  
  return 0;               /* never reached */
}
