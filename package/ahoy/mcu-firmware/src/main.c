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

#define MAX_BRIGHTNESS (99)
#define PWM_STEPS (100)

#define KEY_INPUT       PIND

#define MAIN_BUTTON (1)
#define UP_BUTTON (2)
#define DOWN_BUTTON (4)


volatile uint8_t red = 1;
volatile uint8_t green = 1;
volatile uint8_t blue = 1;

volatile char key_state;                         // debounced and inverted key state: 
                                                 // bit = 1: key pressed 
volatile char key_press;                         // key press detect 

// LED PWM interrupt routine
// on 16-bit timer1
ISR(TIMER1_COMPA_vect)
{
  uint8_t color = 0;
  static uint8_t pwm_index = 0;
  pwm_index++;
  if (pwm_index < red) {
    color = 1;
  }
  if (pwm_index < green) {
    color += 2;
  }
  if (pwm_index < blue) {
    color += 4;
  }
  
  PORTC = color;
}

// button interrupt service routine on 8-bit timer0
ISR(TIMER0_OVF_vect)
{
  static char ct0, ct1; 
  char i; 

  i = key_state ^ ~KEY_INPUT;           // key changed ? 
  ct0 = ~( ct0 & i );                   // reset or count ct0 
  ct1 = ct0 ^ (ct1 & i);                // reset or count ct1 
  i &= ct0 & ct1;                       // count until roll over ? 
  key_state ^= i;                       // then toggle debounced state 
                                        // now debouncing finished 
  key_press |= key_state & i;           // 0->1: key press detect 
}

char get_key_press( char key_mask ) 
{ 
  cli(); 
  key_mask &= key_press;                // read key(s) 
  key_press ^= key_mask;                // clear key(s) 
  sei(); 
  return key_mask; 
} 

int main(void)
{
  cli();             // disable global interrupts

//////////////////////////////////////////////////////////////////////////
// Initialize LEDs
//////////////////////////////////////////////////////////////////////////
  DDRC = 7;           /* make the LED pin an output */
  PORTC= 0; 

  // initialize Timer1
  TCCR1A = 0;        // set entire TCCR1A register to 0
  TCCR1B = 0;


  // set compare match register to desired timer count:
  OCR1A = 65;

  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  
  // Full clock speed (no prescaling)
  TCCR1B |= (1 << CS10);

//////////////////////////////////////////////////////////////////////////
// Initialize Buttons
//////////////////////////////////////////////////////////////////////////

  key_state = 0; 
  key_press = 0; 
  // initialize timer0
  TCCR0A = (1<<CS02);          //divide by 256
  TIMSK0 = 1<<TOIE0;                      //enable timer overflow interrupt 

  DDRD = 0;   // all inputs for now
  PORTD = 0xff;  // enablel pullups
  
  // enable global interrupts:
  sei();
    
//////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////

  for(;;){
    uint8_t buttons = get_key_press(0x07);   // read the lowest three bits
    
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

/*        
      red--;
      red--;
      red--;
      green--;
      green--;
      blue--;
*/
    }

    return 0;               /* never reached */
}
