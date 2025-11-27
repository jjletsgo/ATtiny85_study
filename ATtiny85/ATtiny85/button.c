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

// 장시간 누름 감지용 변수
static timer_ms longPressTimer;            // 장시간 누름 타이머
static Button_t pressedButton = BUTTON_NONE;  // 현재 눌린 버튼
static uint8_t longPressTriggered = 0;     // 긴 누름 이벤트 발생 여부

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
// 버튼별 장시간 누름 기준 시간 반환
// =============================
static uint16_t getLongPressTime(Button_t btn) {
    switch (btn) {
        case BUTTON_1: return LONG_PRESS_MS_BTN1;
        case BUTTON_2: return LONG_PRESS_MS_BTN2;
        case BUTTON_3: return LONG_PRESS_MS_BTN3;
        case BUTTON_4: return LONG_PRESS_MS_BTN4;
        default: return 2000U;  // 기본 2초
    }
}

// =============================
// 버튼 이벤트 감지 (짧은/긴 누름 구분)
// - 긴 누름: 버튼 누르고 있는 동안 기준 시간 경과 시 1회 발생
// - 짧은 누름: 버튼을 뗐을 때, 긴 누름이 발생하지 않았으면 발생
// =============================
ButtonResult_t Button_GetEvent(uint16_t ADC_val) {
    ButtonResult_t result = { BUTTON_NONE, BTN_EVENT_NONE };
    Button_t currentStable = Button_ADC_getDebounced(ADC_val);
    
    // -----------------------
    // 케이스 1: 버튼이 새로 눌림 (NONE -> 버튼)
    // -----------------------
    if (pressedButton == BUTTON_NONE && currentStable != BUTTON_NONE) {
        pressedButton = currentStable;
        longPressTriggered = 0;
        longPressTimer.is_init_done = 0;  // 타이머 시작 준비
        return result;  // 아직 이벤트 없음
    }
    
    // -----------------------
    // 케이스 2: 버튼이 눌려있는 상태
    // -----------------------
    if (pressedButton != BUTTON_NONE && currentStable == pressedButton) {
        // 긴 누름 아직 안 발생했으면 체크
        if (!longPressTriggered) {
            uint16_t longPressTime = getLongPressTime(pressedButton);
            if (timer_delay_ms(&longPressTimer, longPressTime)) {
                // 긴 누름 발생!
                longPressTriggered = 1;
                result.button = pressedButton;
                result.event = BTN_EVENT_LONG_PRESS;
                return result;
            }
        }
        return result;  // 아직 이벤트 없음 (계속 누르고 있음)
    }
    
    // -----------------------
    // 케이스 3: 버튼을 뗌 (버튼 -> NONE)
    // -----------------------
    if (pressedButton != BUTTON_NONE && currentStable == BUTTON_NONE) {
        // 긴 누름이 발생하지 않았으면 짧은 누름 이벤트
        if (!longPressTriggered) {
            result.button = pressedButton;
            result.event = BTN_EVENT_SHORT_PRESS;
        }
        // 상태 초기화
        pressedButton = BUTTON_NONE;
        longPressTriggered = 0;
        longPressTimer.is_init_done = 0;
        return result;
    }
    
    // -----------------------
    // 케이스 4: 다른 버튼으로 바뀜 (드문 케이스)
    // -----------------------
    if (pressedButton != BUTTON_NONE && currentStable != BUTTON_NONE && currentStable != pressedButton) {
        // 이전 버튼 릴리즈 처리
        if (!longPressTriggered) {
            result.button = pressedButton;
            result.event = BTN_EVENT_SHORT_PRESS;
        }
        // 새 버튼으로 전환
        pressedButton = currentStable;
        longPressTriggered = 0;
        longPressTimer.is_init_done = 0;
        return result;
    }
    
    return result;
}

// =============================
// 디바운싱 초기화 (필요시 호출)
// =============================
void Button_Init(void) {
    lastRaw = BUTTON_NONE;
    lastStable = BUTTON_NONE;
    debounceTimer.is_init_done = 0;
    debounceTimer.ms_time = 0;
    longPressTimer.is_init_done = 0;
    longPressTimer.ms_time = 0;
    pressedButton = BUTTON_NONE;
    longPressTriggered = 0;
}
