
#include <util/delay.h>
#include "tm1650.h"

// 7-Segment 숫자 표시 array
const uint8_t SEGMENT_FONT[] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

static uint8_t criticalDotOverride = 0;
static uint8_t criticalDotState = 0;

void I2C_Start(void) {
	I2C_DDR |= (1 << SDA_PIN);   // SDA Output
	I2C_DDR |= (1 << SCL_PIN);   // SCL Output
	
	// 1) SCL: HIGH 상태에서 SDA: HIGH -> LOW
	//
	I2C_PORT |= (1 << SDA_PIN);  // High
	I2C_PORT |= (1 << SCL_PIN);  // High
	_delay_us(4);
	I2C_PORT &= ~(1 << SDA_PIN); // SDA Low (Start Condition)
	_delay_us(4);
	I2C_PORT &= ~(1 << SCL_PIN); // SCL Low
}

void I2C_Stop(void) {
	I2C_DDR |= (1 << SDA_PIN);
	I2C_PORT &= ~(1 << SDA_PIN); // SDA Low
	I2C_PORT |= (1 << SCL_PIN);  // SCL High
	_delay_us(4);
	I2C_PORT |= (1 << SDA_PIN);  // SDA High (Stop Condition)
	_delay_us(4);
}

void I2C_WriteByte(uint8_t data) {
	uint8_t i;
	I2C_DDR |= (1 << SDA_PIN); // SDA Output

	for (i = 0; i < 8; i++) {
		// MSB First
		if (data & 0x80) {
			I2C_PORT |= (1 << SDA_PIN);
			} else {
			I2C_PORT &= ~(1 << SDA_PIN);
		}
		_delay_us(2);
		I2C_PORT |= (1 << SCL_PIN); // Clock High
		_delay_us(2);
		I2C_PORT &= ~(1 << SCL_PIN); // Clock Low
		data <<= 1;
		_delay_us(2);
	}

	// Wait for ACK
	I2C_DDR &= ~(1 << SDA_PIN); // SDA Input
	I2C_PORT |= (1 << SDA_PIN); // Pull-up
	_delay_us(2);
	I2C_PORT |= (1 << SCL_PIN); // Clock High (ACK Check)
	_delay_us(2);
	I2C_PORT &= ~(1 << SCL_PIN); // Clock Low
	I2C_DDR |= (1 << SDA_PIN);   // SDA Output
}

void TM1650_WriteCommand(uint8_t cmd, uint8_t data) {
	I2C_Start();
	I2C_WriteByte(cmd);  // Command (Address)
	I2C_WriteByte(data); // Data
	I2C_Stop();
}


void TM1650_Init(void) {
	// I2C 핀 초기화
	I2C_DDR |= (1 << SDA_PIN) | (1 << SCL_PIN);
	I2C_PORT |= (1 << SDA_PIN) | (1 << SCL_PIN);
	
	// 디스플레이 켜기 및 밝기 설정 (기본 중간 밝기)
	TM1650_WriteCommand(TM1650_CTRL_ADDR, TM1650_BRIGHTNESS_1 | TM1650_DISPLAY_ON);
}

void TM1650_Clear(void) {
	TM1650_WriteCommand(DIGIT1_ADDR, 0x00);
	TM1650_WriteCommand(DIGIT2_ADDR, 0x00);
	TM1650_WriteCommand(DIGIT3_ADDR, 0x00);
	TM1650_WriteCommand(DIGIT4_ADDR, 0x00);
}

void TM1650_DisplayDigit(uint8_t position, uint8_t number, uint8_t dot) {
	uint8_t address = 0;
	uint8_t segment_data = SEGMENT_FONT[number % 10];
	
	if (dot) segment_data |= 0x80;

	switch (position) {
		case 1: address = DIGIT1_ADDR; break; // 천의 자리 (좌측)
		case 2: address = DIGIT2_ADDR; break;
		case 3: address = DIGIT3_ADDR; break;
		case 4: address = DIGIT4_ADDR; break; // 일의 자리 (우측)
		default: return;
	}
	
	TM1650_WriteCommand(address, segment_data);
}

// 0 ~ 9999 숫자 표시 함수
void TM1650_DisplayNumber(int number) {
	if (number > 9999) number = 9999;
	if (number < 0) number = 0;

	uint8_t thousands = number / 1000;
	uint8_t hundreds = (number % 1000) / 100;
	uint8_t tens = (number % 100) / 10;
	uint8_t ones = number % 10;

	
	if (number >= 1000) TM1650_DisplayDigit(1, thousands, 0);
	else TM1650_WriteCommand(DIGIT1_ADDR, 0x00); // 0이면 끄기

	if (number >= 100) TM1650_DisplayDigit(2, hundreds, 0);
	else if (number >= 1000) TM1650_DisplayDigit(2, 0, 0); // 1000이상이면 0 표시
	else TM1650_WriteCommand(DIGIT2_ADDR, 0x00);

	if (number >= 10) TM1650_DisplayDigit(3, tens, 0);
	else if (number >= 100) TM1650_DisplayDigit(3, 0, 0);
	else TM1650_WriteCommand(DIGIT3_ADDR, 0x00);

	TM1650_DisplayDigit(4, ones, 0); // 일의 자리는 항상 표시 (0 포함)
}

// 시간 표시 함수
// Input: 0.1초 단위의 카운트 값 (Max: 5999 = 9분 59.9초)
void TM1650_DisplayTime(uint16_t total_deci_seconds) {
	if (total_deci_seconds > 5999) total_deci_seconds = 5999;

	// 시간 계산
	uint8_t minutes = total_deci_seconds / 600;            // 분
	uint16_t remainder = total_deci_seconds % 600;         // 남은 초(0.1초 단위)
	
	uint8_t seconds_10 = remainder / 100;                  // 초 (10의 자리)
	uint8_t seconds_1  = (remainder % 100) / 10;           // 초 (1의 자리)
	uint8_t decis      = remainder % 10;                   // 0.1초 자리

	// 깜빡임 로직 (Blink Logic)
	// 3번째 자리의 점(Dot)을 1초 주기로 깜빡임 (0.5초 켜짐, 0.5초 꺼짐)
	// decis 값이 0~4일 때는 켜고(1), 5~9일 때는 끔(0)
	uint8_t dot_state = (decis < 5);
	// 화면 출력
	uint8_t digit1_dot = dot_state;
	uint8_t digit3_dot = dot_state;
	if (criticalDotOverride) {
		digit1_dot = criticalDotState;
		digit3_dot = criticalDotState;
	}

	// 1번 자리: 분 (0이어도 표시)
	TM1650_DisplayDigit(1, minutes, digit1_dot);
	
	// 2번 자리: 초 (10의 자리)
	TM1650_DisplayDigit(2, seconds_10, 0);
	
	// 3번 자리: 초 (1의 자리) + 점 깜빡임 적용
	TM1650_DisplayDigit(3, seconds_1, digit3_dot);
	
	// 4번 자리: 0.1초
	TM1650_DisplayDigit(4, decis, 0);
}

// A2: 시한폭탄 종료 효과
void TM1650_BlinkAlarm(void) {
	static uint8_t toggle = 0;
	
	toggle = !toggle; // 호출될 때마다 상태 반전 (0 <-> 1)

	if (toggle) {
		// "0.0.0.0." 처럼 점까지 다 켜서 위급함을 표시하거나
		// 단순히 "0000"을 표시
		TM1650_DisplayDigit(1, 0, 1);
		TM1650_DisplayDigit(2, 0, 1);
		TM1650_DisplayDigit(3, 0, 1);
		TM1650_DisplayDigit(4, 0, 1);
		} else {
		TM1650_Clear();
	}
}

void TM1650_SetCriticalDotState(uint8_t enable, uint8_t state) {
	criticalDotOverride = enable ? 1 : 0;
	criticalDotState = state ? 1 : 0;
}