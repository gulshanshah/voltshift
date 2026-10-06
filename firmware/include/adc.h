#ifndef VS_ADC_H
#define VS_ADC_H

#include <stdint.h>

void vs_adc_init(void);
uint16_t vs_adc_read(uint8_t channel);
uint16_t vs_adc_to_dv(uint8_t channel, uint16_t counts);

#endif
