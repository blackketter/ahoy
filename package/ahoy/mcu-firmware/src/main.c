//
// Firmware for Ahoy AVR MCU 
// Author: Dean Blackketter, Ahoy
// Copyright 2014, All Rights Reserved
//
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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

///////////////////////////////////////////////////////////////////////////////
// LED related defines and globals
///////////////////////////////////////////////////////////////////////////////

#define LED_PORT            PORTD
#define LED_PORT_DIRECTION  DDRD

#define RED_LED (1)
#define GREEN_LED (2)
#define BLUE_LED (4)
#define LED_MASK (0x07)

#define MAX_BRIGHTNESS (99)
#define PWM_STEPS (100)

volatile uint8_t red = 1;
volatile uint8_t green = 1;
volatile uint8_t blue = 1;

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
}

char get_button_press( char button_mask ) 
{ 
  cli(); 
  button_mask &= button_press;                // read button(s) 
  button_press ^= button_mask;                // clear button(s) 
  sei(); 
  return button_mask; 
} 

int main(void)
{
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
  OCR1A = 65;
  
  // Full clock speed (no prescaling)
  TCCR1B |= (1 << CS10);

//////////////////////////////////////////////////////////////////////////
// Initialize Buttons
//////////////////////////////////////////////////////////////////////////

  button_state = 0; 
  button_press = 0; 
  
  // initialize timer0
  TCCR0A = (1<<CS02);          // divide by 256
  TIMSK0 = 1<<TOIE0;           // enable timer overflow interrupt 

  BUTTON_PORT_DIRECTION = 0;   // all inputs for now
  BUTTON_PORT = BUTTON_MASK;   // enable pullups on the buttons
  
  // enable global interrupts:
  sei();
    
//////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////

  for(;;){
  
    uint8_t buttons = get_button_press(BUTTON_MASK);   // read the lowest three bits
    
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
    
    if (buttons & SETUP_BUTTON) {
      if (blue != 0) {
        blue = 0;
        red = 0;
        green = 0;        
      } else {
        blue = 0xff;
        red = 0xff;
        green = 0xff;
      }
    }

  }

  return 0;               /* never reached */
}
