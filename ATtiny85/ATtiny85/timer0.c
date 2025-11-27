/*
 * timer0.c
 *
 * Created: 2025-11-21 오후 10:49:22
 *  Author: User
 * clkT0 is by default connected to the main system clock clkI/O
 */ 

#include "timer0.h"

//프로그램 시작 이후 경과시간을 전역변수로 선언
volatile unsigned long timer0_millis = 0; //누적 시간 (밀리 단위)
volatile uint32_t timer0_micros = 0 ;// 누적 시간 (마이크로 단위. 찌거기로 사용)

//Compare Match Interrupt 
/*
t=(1+OCR0)*N/Fcpu
*/


void init_timer0_FAST_PWM_mode_OVF() { //Timer0-------------->OC0B에 PWM출력. 분주비 8, ovf활성화
	TCCR0A|=(1<< COM0B1) | (1<< WGM01) | (1<< WGM00);
	/*
	Clear OC0A/OC0B on Compare Match, set OC0A/OC0B at BOTTOM
	(non-inverting mode)
	
	Clear OC0A/OC0B on Compare Match, set OC0A/OC0B at BOTTOM
	(non-inverting mode)
	
	FastPWM모드에선 TOP이 0xFF이고 오버플로 인터럽트 플래그가 0XFF에서 발생
	
	*/
	TCCR0B |= (1<< CS01); // set prescaler value to 8 -> io 클럭을 8로 분주해서 사용
	OCR0B = 0;
	TIMSK |= (1<<TOIE0); //Timer/Counter0 Overflow Interrupt Enable
	DDRB |= (1<<PB1); //PWM OUTPUT ENABLE
	sei();
}

// 오버플로우가 몇초마다 발생하는지 아니까 그걸 이용해서 오버플로우 발생시마다 경과시간 누적
// 즉, 인터럽트가 주기적으로 계속 발생하며 누적 경과 시간을 계속해서 업데이트함
ISR(TIMER0_OVF_vect) {
	unsigned long m = timer0_millis;
	uint32_t f = timer0_micros;
	
	m += MILLIS_INCREMENT_PER_OVERFLOW; //Timer0 오버플로우 발생시마다 걸린 밀리초(몫)단위 누적. 누적(경과) 밀리초
	f += MICROS_INCREMENT_PER_OVERFLOW; //Timer0 오버플로우 발생시마다 걸린 마이크로초단위(나머지) 누적. 누적(경과) 마이크로초
	// 이 몫과 나머지를 전부 더한게 실제로 오버플로 인터럽트 1회 발생시까지 걸리는 시간이다. 즉 카운트가 0~255 까지 총 256개의 수 셀때까지 걸리는 시간.
	
	// 오버플로 인터럽트 발생시마다 누적해온 마이크로초가(나머지,찌꺼기) 1000을 넘어가면 밀리초 단위로 변환
	int micro_to_millis = f / 1000;
	// 앞서 밀리초로 변환한 값을 누적 밀리초에 더함.
	m += micro_to_millis;
	
	// 앞서 밀리초로 변환한 만큼을 f에서 빼준다.
	f = f % 1000;
	
	// 외부로 노출되는 전역 변수에 ISR에서 수정한 누적(경과) 시간 값을 반영해준다.
	timer0_millis = m;
	timer0_micros = f;

}

unsigned long millis() {
	unsigned long m;
	uint8_t oldSREG = SREG; //SREG 상태 레지스터값을 저장
	
	cli(); // SREG 레지스터의 I를 0으로 clear하여 인터럽트 비활성화
	
	m = timer0_millis; // m에 오버플로 인터럽트로 계산한 경과 밀리초 저장
	
	SREG = oldSREG; // 다시 인터럽트 활성화
	
	return m; // 수정한 경과 밀리초 반환
}

//atomic하게 s단위의 누적 경과 시간 값을 읽어서 반환하는 함수
uint16_t secs() {
	return (uint16_t)(millis()/1000);
}

//To. 민규씨, 상민씨. 인자로 타이머의 주소와 , ms 단위의 딜레이 간격을 주시면됩니다. 딜레이는 16비트 숫자 전부 가능합니다!
uint8_t timer_delay_ms(timer_ms *timer, uint16_t delay_ms)
{
	unsigned long now = millis();   // 한번만 읽어두고 재사용
	// 아직 타이머가 초기화 안 됐으면, 기준점만 잡고 리턴 0
	if (!timer->is_init_done) {
		timer->ms_time = now;
		timer->is_init_done = 1;
		return 0;
	}
	// 초기화된 상태에서, 설정한 시간만큼 지났으면
	if ((unsigned long)(now - timer->ms_time) >= delay_ms) {
		timer->ms_time = now;   // 다음 주기 기준점 갱신
		return 1;               // 한 번 1을 뿜어줌
	}
	return 0;
}
