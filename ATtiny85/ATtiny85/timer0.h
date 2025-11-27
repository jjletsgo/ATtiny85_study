/*
 * timer0.h
 *
 * Created: 2025-11-21 오후 10:54:59
 *  Author: User
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_
#include "common.h"
//1 마이크로초당 클럭이 몇 주기 들어오는지 
// 1 마이크로초당 클럭 주기 수
#define CLOCKS_PER_MICRO ( F_CPU / 1000000UL )

#define PRESCALE_VALUE_TIMER0 8UL
#define TIMER0_SIZE    256UL

// 클럭수를 마이크로초로 변환 (32비트로 강제)
#define CLOCKS_TO_MICROSECONDS(a) ( ((uint32_t)(a)) / CLOCKS_PER_MICRO )

// Timer0 오버플로까지 걸리는 마이크로초
#define MICROSECONDS_PER_TIMER0_OVERFLOW \
( CLOCKS_TO_MICROSECONDS((uint32_t)PRESCALE_VALUE_TIMER0 * (uint32_t)TIMER0_SIZE) )

// 오버플로 증가량 (밀리 초 단위)
#define MILLIS_INCREMENT_PER_OVERFLOW  ( MICROSECONDS_PER_TIMER0_OVERFLOW / 1000 )

// 오버플로 증가량 (마이크로초 잔여)
#define MICROS_INCREMENT_PER_OVERFLOW  ( MICROSECONDS_PER_TIMER0_OVERFLOW % 1000 )




/*8bit Fast PWM MODE쓰는 경우*/
#define DUTY_CYCLE 50UL //%단위로 0%~100% 사이로 Duty Cycle을 적어주세요.
#define OCR0B_VAL_FOR_8_BIT_FAST_PWM  ( (uint16_t)( (255UL * DUTY_CYCLE / 100UL) - 1UL ) )
/*8bit Fast PWM MODE쓰는 경우*/



typedef struct {
	uint16_t is_init_done; // 첫 번째 호출인지 여부확인용 플래그변수 (C99 표준에서는 초기값 설정 불가)
	unsigned long ms_time; // ms 시간 저장할 변수
} timer_ms;

// 함수 프로토타입
uint8_t timer_delay_ms(timer_ms *timer, uint16_t delay_ms); //초기화 함수 꼭 호출해주기!!!!!!!!!!!!!!!!!!!!
void init_timer0_FAST_PWM_mode_OVF() ; //초기화 함수 꼭 호출해주기!!!!!!!!!!!!!!!!!!!!
unsigned long millis(); 
uint16_t secs();  // secs() 함수 선언 추가

#endif /* TIMER0_H_ */
