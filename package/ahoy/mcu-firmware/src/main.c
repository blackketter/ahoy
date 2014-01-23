#include <avr/io.h>
#include <util/delay.h>

#define MAX_BRIGHTNESS (699)
#define PWM_STEPS (700)

// bigger is slower
#define BRIGHTNESS_RAMP_SPEED (1)

int main(void)
{
    DDRA = 7;           /* make the LED pin an output */
    PORTA= 0; 
    char dir = 1;
    
    uint16_t pwm_index = 0;
    uint16_t brightness = 0;
    uint8_t  k = 0;
    uint8_t  color = 1;
    for(;;){

      pwm_index++;
      
      if (pwm_index > PWM_STEPS) {
        pwm_index = 0;
        k++;
      }
      
      if (k > BRIGHTNESS_RAMP_SPEED) {
        k = 0;
        
        brightness += dir;

      }
      
      if (brightness > MAX_BRIGHTNESS) {
        dir = -1;
      } else if (brightness == 0) {
        dir = 1;
        color++;
        if (color > 7) {
          color = 1;
        }
      }

      if (brightness > pwm_index){
        PORTA = color;
      } else {
        PORTA = 0;
      }
    }
    return 0;               /* never reached */
}
