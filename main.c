/* main.c */
#define F_CPU 8000000UL	// CKDIV7
#include <avr/io.h>
#include <util/delay.h>
#include "tm1650.h"

//
//#define BUTTON_START  (PINB & (1 << PB3)) 
//#define BUTTON_RESET  (PINB & (1 << PB4))

int main(void)
{
    TM1650_Init();
    TM1650_Clear();

    // 초기 설정 시간: 1분 (60.0초 = 600)
    // 과제 요구사항의 'Time Set' 기능은 버튼 입력으로 이 값을 변경하면 됨
    uint16_t timer_count = 600; 
    
    // 동작 상태 변수
    // 0: Ready
    // 1: Running
    // 2: Blink
    uint8_t state = 0; 

    // 버튼 눌림 방지용 변수
    //uint8_t btn_prev = 0;

    while (1) 
    {
        // ----------------------------------------
        // 1. 상태별 동작 및 디스플레이
        // ----------------------------------------
        if (state == 2) {
            TM1650_BlinkAlarm();
            _delay_ms(200);
        } 
        else {
            TM1650_DisplayTime(timer_count);
            
            if (state == 1) {
                // 카운트 다운 로직
                if (timer_count > 0) {
                    timer_count--;
                    _delay_ms(98); // 0.1초 시간 지연 (실행시간 보정)
                } else {
                    // 시간이 0이 되면 폭발 상태로 변경
                    state = 2; 
                }
            } else {
                // 대기 상태면 딜레이만 줌 (버튼 반응 속도 유지)
                _delay_ms(100);
            }
        }

        // ----------------------------------------
        // 2. 버튼 입력 처리 (간단 예시)
        // ----------------------------------------
        // 여기서는 버튼 하나로 Start/Pause를 토글하는 로직 예시입니다.
        
        // 버튼이 눌렸는지 확인 (Active Low/High 회로에 따라 조건 수정 필요)
        // 예: 풀업 저항 사용 시 !BUTTON_START
        /*
        if (BUTTON_START && (btn_prev == 0)) { // 버튼을 막 누른 순간
            if (state == 0) state = 1;      // 대기 -> 시작
            else if (state == 1) state = 0; // 시작 -> 일시정지
            else if (state == 2) {          // 폭발 -> 리셋
                state = 0;
                timer_count = 600; // 시간 초기화
            }
        }
        btn_prev = BUTTON_START; // 버튼 상태 저장
        */
       
       // 테스트를 위해 강제로 시작 (버튼 구현 전 테스트용)
       if(state == 0) state = 1; 
    }
}