/*
 * button.c
 *
 * Created: 2025-09-15 오후 6:54:13
 *  Author: User
 */ 

#include "button.h"
#include "timer0.h"   // timer_ms, timer_delay_ms 사용하려면 필요함

// 정적 변수들 (디바운싱을 위한 상태 저장)
static Button_t lastRaw    = BUTTON_NONE;  // 마지막으로 읽은 Raw 버튼
static Button_t lastStable = BUTTON_NONE;  // 디바운싱 후 안정된 버튼
static timer_ms debounceTimer;             // 디바운싱용 타이머

// =============================
// 버튼 상태 판별 (Raw ADC 값)
// =============================
Button_t Button_ADC_getRaw(uint16_t ADC_val) {
    if (ADC_val >= BUTTON_1_MIN && ADC_val <= BUTTON_1_MAX) {
        return BUTTON_1;
    } 
    else if (ADC_val >= BUTTON_2_MIN && ADC_val <= BUTTON_2_MAX) {
        return BUTTON_2;
    } 
    else if (ADC_val >= BUTTON_3_MIN && ADC_val <= BUTTON_3_MAX) {
        return BUTTON_3;
    } 
    else if (ADC_val >= BUTTON_4_MIN && ADC_val <= BUTTON_4_MAX) {
        return BUTTON_4;
    } 
    else {
        return BUTTON_NONE;
    }
}

// =============================
// 디바운스 처리된 버튼 판별
//  - 같은 Raw 값이 BUTTON_DEBOUNCE_MS ms 동안 유지되면
//    그때 비로소 lastStable 갱신함
// =============================
Button_t Button_ADC_getDebounced(uint16_t ADC_val) {
    Button_t currentRaw = Button_ADC_getRaw(ADC_val);

    // Raw 값이 바뀐 경우: 타이머 리셋 (새로 디바운싱 시작)
    if (currentRaw != lastRaw) {
        lastRaw = currentRaw;
        // 다음 timer_delay_ms 호출에서 기준점 다시 잡도록 초기화
        debounceTimer.is_init_done = 0;
        return lastStable; // 아직 안정됐다고 보기 전에 이전 값 유지
    }

    // Raw 값이 그대로인 경우: 지정한 ms 동안 유지됐는지 확인
    if (timer_delay_ms(&debounceTimer, BUTTON_DEBOUNCE_MS)) {
        // BUTTON_DEBOUNCE_MS ms 동안 currentRaw가 유지됐으므로
        // 안정된 값으로 인정
        lastStable = currentRaw;
    }

    return lastStable;
}

// =============================
// 버튼 눌림 감지 (Edge Detection)
//  - "NONE -> 어떤 버튼"으로 바뀌는 순간만 이벤트 발생
// =============================
Button_t Button_ADC_getPressed(uint16_t ADC_val) {
    static Button_t previousStable = BUTTON_NONE;
    
    Button_t currentStable = Button_ADC_getDebounced(ADC_val);
    
    // 이전 상태는 NONE, 현재는 버튼이면 → 이번에 새로 눌린 이벤트
    if (previousStable == BUTTON_NONE && currentStable != BUTTON_NONE) {
        previousStable = currentStable;
        return currentStable;
    }
    
    // 상태 업데이트
    previousStable = currentStable;
    
    return BUTTON_NONE;  // 새 눌림 이벤트 없으면 NONE
}

// =============================
// 디바운싱 초기화 (필요시 호출)
// =============================
void Button_Init(void) {
    lastRaw = BUTTON_NONE;
    lastStable = BUTTON_NONE;
    debounceTimer.is_init_done = 0;
    debounceTimer.ms_time = 0;
}
