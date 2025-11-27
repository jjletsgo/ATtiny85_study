
#ifndef ADC_H_
#define ADC_H_

#include "common.h"
#define ADC_PIN PB3

void ADC_ch1_init(void);
void read_adc_val (uint16_t *adc_val);



#endif /* ADC_H_ */



