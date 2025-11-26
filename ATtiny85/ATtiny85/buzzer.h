
#ifndef BUZZER_H_
#define BUZZER_H_

#include <avr/io.h>
#include <stdint.h>
#include "timer0.h"   

#define BUZZER_DDR   DDRB
#define BUZZER_PORT  PORTB
#define BUZZER_PIN   PB1

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);

void buzzer_on(uint8_t duty);

void buzzer_off(void);

void buzzer_beep_toggle_periodic(uint16_t period_ms, uint8_t duty);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H_ */