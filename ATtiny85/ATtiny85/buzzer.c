

#include "buzzer.h"

uint16_t buzzer_duration = 0;
timer_ms buzzer_timer;          // 부저용 별도 타이머 추가

void buzzer_on(uint8_t duty)
{
	BUZZER_DDR |= (1 << BUZZER_PIN);
	TCCR0A |= (1 << COM0B1);
    OCR0B = duty;   

}

void buzzer_off(void)
{
    OCR0B = 0;   
	BUZZER_DDR &= ~(1 << BUZZER_PIN);
}


void buzzer_work(void){
	
	if(buzzer_duration && (timer_delay_ms(&buzzer_timer, 5))){
		buzzer_on(200);
		buzzer_duration -= 5;
	}
	else if (!buzzer_duration) buzzer_off();
}

void set_buzzer_duration() {
	buzzer_duration = 3000;
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