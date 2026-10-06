#ifndef VS_SELECTOR_H
#define VS_SELECTOR_H

#include <stdint.h>

void vs_selector_init(void);
void vs_selector_sample(void);
void vs_selector_evaluate(uint8_t startup);
uint8_t vs_selector_position(void);
uint8_t vs_selector_valid(uint8_t channel);
uint16_t vs_selector_dv(uint8_t channel);
uint8_t vs_selector_mask(void);

#endif
