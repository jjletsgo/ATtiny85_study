/*
 * tm1650.h
 * ATtiny85 TM1650 Library Header
 */ 

#ifndef TM1650_H_
#define TM1650_H_

#include <avr/io.h>

// --- 사용자 설정 (Pin Configuration) ---
#define I2C_PORT    PORTB
#define I2C_DDR     DDRB
#define I2C_PIN     PINB
#define SDA_PIN     PB0
#define SCL_PIN     PB2

// --- 상수 정의 ---
#define TM1650_DISPLAY_BASE 0x34 // 기본 제어 커맨드 (예: 0x24<<1) -> I2C 주소 체계에 따라 다름, TM1650은 고정 커맨드 사용
// TM1650 커맨드 (Dig1~4 주소)
#define DIGIT1_ADDR 0x68
#define DIGIT2_ADDR 0x6A
#define DIGIT3_ADDR 0x6C
#define DIGIT4_ADDR 0x6E

// 밝기 설정 및 On/Off (0x48)
#define TM1650_CTRL_ADDR 0x48
#define TM1650_BRIGHTNESS_1  0x11
#define TM1650_BRIGHTNESS_8  0x71 // 최대 밝기
#define TM1650_DISPLAY_ON    0x01
#define TM1650_DISPLAY_OFF   0x00

// --- 함수 선언 ---
void TM1650_Init(void);
void TM1650_Clear(void);
void TM1650_SetBrightness(uint8_t brightness_level);
void TM1650_DisplayDigit(uint8_t position, uint8_t number, uint8_t dot);
void TM1650_DisplayNumber(int number); // 0 ~ 9999 표시 함수
void TM1650_DisplayTime(uint16_t total_deci_seconds);
void TM1650_BlinkAlarm(void);
void TM1650_SetCriticalDotState(uint8_t enable, uint8_t state);

#endif /* TM1650_H_ */