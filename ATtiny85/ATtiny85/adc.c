
#include "ADC.h"

void ADC_ch1_init(void) {
	ADMUX |= (1<<MUX0); //Vref를 AVCC with external capacitor at AREF pin 로 설정, 채널을 ADC1로 선택, 우측정렬
	ADCSRA |= (1 << ADEN) | (1 << ADATE) | (1 << ADPS2) | ( 1<< ADSC); //ADC enable, ADC 자동 트리거 활성화, ADC 변환 시작, SYS클럭을 16으로 분주해서 사용
	DDRB &= ~ (1<<PB2); //PB2=ADC1을 입력으로 설정	

}



void read_adc_val (uint16_t *adc_val) {
	uint8_t Low_Byte = ADCL; //8bit read이니 자동 atomic함. ADCL 읽는 순간 ADCH는 LOCK됨.(ADC가 새로운 값 읽어도, 이 동안은 잠시 UPDATE 안된다는 뜻)
	uint8_t High_Byte = ADCH; //ADCH 읽었으니 ADCH LOCK 해제
	*adc_val = (((uint16_t)High_Byte) << 8) | Low_Byte;
}



