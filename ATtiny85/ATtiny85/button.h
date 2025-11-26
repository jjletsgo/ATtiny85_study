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
#define BUTTON_DEBOUNCE_MS   20U

// =============================
// 아날로그 입력 범위 (필요시 조정)
// =============================
#define BUTTON_1_MIN         0U
#define BUTTON_1_MAX        50U

#define BUTTON_2_MIN        70U
#define BUTTON_2_MAX       150U

#define BUTTON_3_MIN       205U
#define BUTTON_3_MAX       280U

#define BUTTON_4_MIN       285U
#define BUTTON_4_MAX       350U

// 버튼 타입 정의
typedef enum {
    BUTTON_NONE = 0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
} Button_t;

// 함수 프로토타입
Button_t Button_ADC_getRaw(uint16_t ADC_val);      // Raw ADC 값 -> 버튼 상태
Button_t Button_ADC_getDebounced(uint16_t ADC_val);// 디바운싱된 현재 버튼 상태
Button_t Button_ADC_getPressed(uint16_t ADC_val);  // "한 번 눌림" 이벤트 감지
void Button_Init(void);                            // 초기화

#endif /* BUTTON_H_ */
