

#include "buzzer.h"

void buzzer_init(void)
{
    BUZZER_DDR |= (1 << BUZZER_PIN);

    OCR0B = 0; 
}

void buzzer_on(uint8_t duty)
{
    OCR0B = duty;   

}

void buzzer_off(void)
{
    OCR0B = 0;   
}


void buzzer_beep_toggle_periodic(uint16_t period_ms, uint8_t duty)
{
    static timer_ms beep_timer = {0};
    static uint8_t  buzzer_state = 0; 

    if (timer_delay_ms(&beep_timer, period_ms)) {
        buzzer_state = !buzzer_state;

        if (buzzer_state) {
            buzzer_on(duty);
        } else {
            buzzer_off();
        }
    }
}