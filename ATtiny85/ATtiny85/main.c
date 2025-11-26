/*
 * ATtiny85.c
 *
 * Created: 2025-11-26 오전 9:48:35
 * Author : User
 */

#include "common.h"

int main(void)
{
    init_timer0_FAST_PWM_mode_OVF();  // millis() 타이머
    ADC_ch1_init();                   // ADC 채널1 초기화
    Button_Init();                    // 버튼 디바운싱 초기화
    sei();

    uint16_t adc_val;
    Button_t btn;

    DDRB |= (1 << 4);   // PB4 출력 설정

    while (1)
    {
        read_adc_val(&adc_val);

        btn = Button_ADC_getPressed(adc_val);

        if (btn != BUTTON_NONE) 
        {
            switch (btn)
            {
                case BUTTON_1:
                    //PINB |= (1 << 4);   // PB4 토글 (LED 점멸용)
                    break;

                case BUTTON_2:
                    // TODO: 버튼2 동작
					PINB |= (1 << 4);   // PB4 토글 (LED 점멸용)
                    break;
	
                case BUTTON_3:
                    // TODO: 버튼3 동작
                    break;

                case BUTTON_4:
                    // TODO: 버튼4 동작
                    break;

                default:
                    break;
            }
        }
    }
}
