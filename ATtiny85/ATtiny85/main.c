/*
 * ATtiny85.c
 *
 * Created: 2025-11-26 오전 9:48:35
 * Author : User
 */ 

#include"common.h"


int main(void)
{
	timer_ms led_timer;
	init_timer0_FAST_PWM_mode_OVF();
	sei();
	DDRB |= (1<<PB4);
    /* Replace with your application code */
    while (1) 
    {
		//if(timer_delay_ms(&led_timer, 500)) PINB=(1<<PB4);
    }
}

