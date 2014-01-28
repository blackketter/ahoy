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


volatile uint8_t red = 0;
volatile uint8_t green = 0;
volatile uint8_t blue = 0;

// LED PWM interrupt routine
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

int main(void)
{
    DDRC = 7;           /* make the LED pin an output */
    PORTC= 0; 

    // initialize Timer1
    cli();             // disable global interrupts
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
    
    // enable global interrupts:
    sei();
    
    for(;;){
      red--;
      red--;
      red--;
      green--;
      green--;
      blue--;
      _delay_ms(4);
    }

    return 0;               /* never reached */
}
