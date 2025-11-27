/*
 * button.h
 *
 * Created: 2025-09-15 오후 6:54:13
 *  Author: User
 */ 

#ifndef BUTTON_H_
#define BUTTON_H_

#include "common.h"

// 디바운스 시간 (ms 단위) - 필요하면 10~30 사이로 조절
#define BUTTON_DEBOUNCE_MS   2U


#define BUTTON_1_MIN         0U
#define BUTTON_1_MAX        40U

#define BUTTON_2_MIN        55U
#define BUTTON_2_MAX       140U
#define BUTTON_3_MIN       200U
#define BUTTON_3_MAX       300U

#define BUTTON_4_MIN       400U
#define BUTTON_4_MAX       500U

// =============================
// 장시간 누름 감지용 시간 (ms)
// =============================
#define LONG_PRESS_MS_BTN1   1500U   // 버튼1: 1.5초
#define LONG_PRESS_MS_BTN2   2000U   // 버튼2: 2초
#define LONG_PRESS_MS_BTN3   1000U   // 버튼3: 1초
#define LONG_PRESS_MS_BTN4   1000U   // 버튼4: 1초

// 버튼 타입 정의
typedef enum {
    BUTTON_NONE = 0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
} Button_t;

// 버튼 이벤트 타입
typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT_PRESS,   // 짧게 눌렀다 뗌
    BTN_EVENT_LONG_PRESS,    // 길게 누름 (누르고 있는 동안 1회 발생)
} ButtonEvent_t;

// 버튼 이벤트 결과 구조체
typedef struct {
    Button_t button;         // 어떤 버튼인지
    ButtonEvent_t event;     // 이벤트 종류
} ButtonResult_t;

// 함수 프로토타입
Button_t Button_ADC_getRaw(uint16_t ADC_val);       // Raw ADC 값 -> 버튼 상태
Button_t Button_ADC_getDebounced(uint16_t ADC_val); // 디바운싱된 현재 버튼 상태
ButtonResult_t Button_GetEvent(uint16_t ADC_val);   // 버튼 이벤트 감지 (짧은/긴 누름)
void Button_Init(void);                             // 초기화

#endif /* BUTTON_H_ */
