#ifndef VS_STEPPER_H
#define VS_STEPPER_H

#include <stdint.h>

void vs_stepper_init(void);
void vs_stepper_goto(uint8_t position);
void vs_stepper_tick(uint32_t now);
uint8_t vs_stepper_busy(void);
uint8_t vs_stepper_position(void);

#endif
