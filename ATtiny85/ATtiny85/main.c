// ATtiny85 stopwatch/bomb 모드 메인 코드입니다.

#include "common.h"
#include "tm1650.h"

// 모드, 상태 enum 묶어둘게요.
typedef enum {
	MODE_STOPWATCH = 0,  // 스톱워치 모드
	MODE_BOMB            // 시한폭탄 모드
} AppMode_t;

typedef enum {
	STATE_STOPPED = 0,   // 정지 상태
	STATE_RUNNING,       // 동작 중
	STATE_EXPLODED       // 폭발 상태 (폭탄 모드 전용)
} RunState_t;

// 동작에 쓰는 전역 값들입니다.
static AppMode_t  currentMode  = MODE_STOPWATCH;
static RunState_t runState     = STATE_STOPPED;
static uint16_t   timeCounter  = 0;      // 0.1초 단위 (스톱워치: 증가, 폭탄: 감소)
static uint16_t   bombSetTime  = 300;    // 폭탄 모드 기본 설정 시간 (30.0초 = 300 * 0.1초)

// 0.1초 단위 타이머
static timer_ms   timerTick;

// 폭발 깜빡임 타이머
static timer_ms   blinkTimer;

// 부저랑 LED 상태 추적
static timer_ms   buzzerTimer;
static uint8_t    buzzerIsOn = 0;
// 부저는 짧게만 울려주세요.
#define BUZZER_PULSE_MS   80U
// LED는 부저보다 살짝 더 오래 켜둘게요.
static timer_ms   ledTimer;
static uint8_t    ledIsOn = 0;
#define LED_PULSE_MS      150U
#define EXPLOSION_PERIOD_MS 50U

// 부저 duty 조정용 헬퍼입니다.
static void Buzzer_SetLevel(uint8_t level)
{
	if (level == 0) {
		OCR0B = 0;        // 듀티 0% → 소리 끔
		return;
	}

	uint16_t val = (255UL * (uint16_t)level) / 100UL;
	if (val > 0) val -= 1;
	if (val > 255) val = 255;
	OCR0B = (uint8_t)val;
}

// 폭탄 모드 비프/LED 패턴은 여기서 다룹니다.
static void Buzzer_Update(void)
{
	static uint8_t criticalDotEnabled = 0;
	static uint8_t criticalDotState = 0;

	if (runState == STATE_EXPLODED) {
		uint16_t wait_ms = buzzerIsOn ? BUZZER_PULSE_MS
		: ((EXPLOSION_PERIOD_MS > BUZZER_PULSE_MS) ? (EXPLOSION_PERIOD_MS - BUZZER_PULSE_MS) : 10);
		if (timer_delay_ms(&buzzerTimer, wait_ms)) {
			if (!buzzerIsOn) {
				Buzzer_SetLevel(65);
				buzzerIsOn = 1;
				} else {
				Buzzer_SetLevel(0);
				buzzerIsOn = 0;
			}
		}
		return;
	}

	// 폭탄 모드 아니면 부저/LED 꺼주세요.
	if (currentMode != MODE_BOMB || runState != STATE_RUNNING) {
		Buzzer_SetLevel(0);
		buzzerIsOn = 0;
		buzzerTimer.is_init_done = 0;
		// LED도 끔
		PORTB &= ~(1 << 4);
		ledIsOn = 0;
		ledTimer.is_init_done = 0;
		if (criticalDotEnabled) {
			criticalDotEnabled = 0;
			criticalDotState = 0;
			TM1650_SetCriticalDotState(0, 0);
		}
		return;
	}

	uint16_t seconds = timeCounter / 10; // 남은 초 (정수)
	uint16_t period_ms;  // 삡~삡 사이 간격 (주기)
	uint8_t  volume;     // 볼륨

	uint8_t inCriticalWindow = (seconds <= 5);
	if (!inCriticalWindow && criticalDotEnabled) {
		criticalDotEnabled = 0;
		criticalDotState = 0;
		TM1650_SetCriticalDotState(0, 0);
	}

	if (seconds > 5) {
		// 여유 있을 땐 1초마다 한번만 울려주세요.
		period_ms = 1000;
		volume    = 35;
	}
	else if (seconds > 3) { // 4~5초
		// 슬슬 급해지니 0.65초 간격으로 바꿉니다.
		period_ms = 650;
		volume    = 45;
	}
	else if (seconds > 1) { // 2~3초
		// 더 촉박하니 0.45초 간격으로.
		period_ms = 450;
		volume    = 55;
	}
	else {                  // 0~1초
		// 마지막 구간은 0.28초 정도로 급하게.
		period_ms = 280;
		volume    = 65;
	}

	// ON 상태에서는 BUZZER_PULSE_MS 후 바로 OFF,
	// OFF 상태에서는 (period_ms - BUZZER_PULSE_MS) 만큼 쉬었다가 다시 ON 시작
	uint16_t wait_ms = buzzerIsOn ? BUZZER_PULSE_MS
	: ( (period_ms > BUZZER_PULSE_MS) ? (period_ms - BUZZER_PULSE_MS) : 10 );

	if (timer_delay_ms(&buzzerTimer, wait_ms)) {
		if (!buzzerIsOn) {
			// 삡 시작할 때 duty 올려주세요.
			Buzzer_SetLevel(volume);
			buzzerIsOn = 1;

			// LED도 같이 켜두고 타이머 초기화해주세요.
			PORTB |= (1 << 4);
			ledIsOn = 1;
			ledTimer.is_init_done = 0;

			if (inCriticalWindow) {
				criticalDotEnabled = 1;
				criticalDotState ^= 1;
				TM1650_SetCriticalDotState(1, criticalDotState);
			}
			} else {
			// 울림 끝났으면 바로 duty 내립니다.
			Buzzer_SetLevel(0);
			buzzerIsOn = 0;
		}
	}

	// LED는 지정 시간 지나면 꺼주세요.
	if (ledIsOn && timer_delay_ms(&ledTimer, LED_PULSE_MS)) {
		PORTB &= ~(1 << 4);
		ledIsOn = 0;
	}
}

int main(void)
{
	init_timer0_FAST_PWM_mode_OVF();  // millis() 타이머
	ADC_ch1_init();                   // ADC 채널1 초기화
	Button_Init();                    // 버튼 디바운싱 초기화
	TM1650_Init();                    // 7세그먼트 디스플레이 초기화
	sei();

	uint16_t adc_val;
	ButtonResult_t btnResult;

	DDRB |= (1 << 4);   // PB4 출력 설정

	// 초기 화면 표시
	TM1650_DisplayTime(timeCounter);

	while (1)
	{
		read_adc_val(&adc_val);
		
		// 폭발 상태에서는 계속 경보 울리고 깜빡이게 해주세요.
		if (runState == STATE_EXPLODED) {
			PORTB |= (1 << 4); // LED 계속 켬

			if (timer_delay_ms(&blinkTimer, 250)) {
				TM1650_BlinkAlarm();
			}
			
			// 버튼 아무거나 누르면 초기 상태로 돌려주세요.
			btnResult = Button_GetEvent(adc_val);
			if (btnResult.event != BTN_EVENT_NONE) {
				// 부저/LED 모두 끔
				Buzzer_SetLevel(0);
				buzzerIsOn = 0;
				buzzerTimer.is_init_done = 0;
				PORTB &= ~(1 << 4);
				ledIsOn = 0;
				ledTimer.is_init_done = 0;

				currentMode = MODE_STOPWATCH;
				runState = STATE_STOPPED;
				timeCounter = 0;
				TM1650_DisplayTime(timeCounter);
			}
			Buzzer_Update();
			continue;
		}

		// 버튼 이벤트 받아서 모드별로 처리합니다.
		btnResult = Button_GetEvent(adc_val);

		if (btnResult.event != BTN_EVENT_NONE)
		{
			switch (btnResult.button)
			{
				// 버튼1은 모드 바꿀 때만 씁니다.
				case BUTTON_1:
				if (btnResult.event == BTN_EVENT_SHORT_PRESS) {
					// 모드 전환
					if (currentMode == MODE_STOPWATCH) {
						currentMode = MODE_BOMB;
						timeCounter = bombSetTime;
						} else {
						currentMode = MODE_STOPWATCH;
						timeCounter = 0;
					}
					runState = STATE_STOPPED;
					timerTick.is_init_done = 0;
					TM1650_DisplayTime(timeCounter);
				}
				break;

				// 버튼2는 시작/정지와 모드 유지 리셋 전용입니다.
				case BUTTON_2:
				if (btnResult.event == BTN_EVENT_SHORT_PRESS) {
					// 시작/일시정지 토글 (모드 공통)
					if (runState == STATE_STOPPED) {
						runState = STATE_RUNNING;
						timerTick.is_init_done = 0;
						} else {
						runState = STATE_STOPPED;
					}
				}
				else if (btnResult.event == BTN_EVENT_LONG_PRESS) {
					// 길게: 모드 유지 리셋
					runState = STATE_STOPPED;
					timerTick.is_init_done = 0;
					if (currentMode == MODE_STOPWATCH) {
						timeCounter = 0;
						} else {
						timeCounter = bombSetTime; // 폭탄 모드: 설정 시간으로 리셋
					}
					TM1650_DisplayTime(timeCounter);
				}
				break;

				// 버튼3은 폭탄 시간 늘릴 때만 봐주세요.
				case BUTTON_3:
				if (currentMode == MODE_BOMB && runState == STATE_STOPPED) {
					if (btnResult.event == BTN_EVENT_SHORT_PRESS) {
						// +1초 = +10 (0.1초 단위)
						timeCounter += 10;
						if (timeCounter > 5999) timeCounter = 5999;
					}
					else if (btnResult.event == BTN_EVENT_LONG_PRESS) {
						// +10초 = +100 (0.1초 단위)
						timeCounter += 100;
						if (timeCounter > 5999) timeCounter = 5999;
					}
					TM1650_DisplayTime(timeCounter);
				}
				// 스톱워치 모드에서는 무시
				break;

				// 버튼4는 폭탄 시간 줄일 때만 사용합니다.
				case BUTTON_4:
				// 시한폭탄 모드 + 정지 상태에서만 동작
				if (currentMode == MODE_BOMB && runState == STATE_STOPPED) {
					if (btnResult.event == BTN_EVENT_SHORT_PRESS) {
						// -1초 = -10 (0.1초 단위)
						if (timeCounter >= 10) {
							timeCounter -= 10;
							} else {
							timeCounter = 0;
						}
					}
					else if (btnResult.event == BTN_EVENT_LONG_PRESS) {
						// -10초 = -100 (0.1초 단위)
						if (timeCounter >= 100) {
							timeCounter -= 100;
							} else {
							timeCounter = 0;
						}
					}
					TM1650_DisplayTime(timeCounter);
				}
				// 스톱워치 모드 또는 동작 중에는 무시
				break;

				default:
				break;
			}
		}

		// 0.1초 타이밍 맞춰서 카운터 업데이트합니다.
		if (runState == STATE_RUNNING) {
			if (timer_delay_ms(&timerTick, 100)) {  // 100ms = 0.1초
				if (currentMode == MODE_STOPWATCH) {
					// 스톱워치: 시간 증가
					if (timeCounter < 5999) {
						timeCounter++;
					}
				}
				else {
					// 시한폭탄: 시간 감소
					if (timeCounter > 0) {
						timeCounter--;
					}
					else {
						// 0이 되면 폭발!
						runState = STATE_EXPLODED;
						blinkTimer.is_init_done = 0;
						// 폭발 직후부터 LED 켜두고 비프 패턴은 별도 루틴에서 처리
						Buzzer_SetLevel(0);
						buzzerIsOn = 0;
						buzzerTimer.is_init_done = 0;
						// LED는 계속 켠 상태 유지
						PORTB |= (1 << 4);
						ledIsOn = 0;
						ledTimer.is_init_done = 0;
						TM1650_SetCriticalDotState(0, 0);
					}
				}
				TM1650_DisplayTime(timeCounter);
			}
		}

		// 비프/LED 패턴 갱신 루틴
		Buzzer_Update();
	}
}
