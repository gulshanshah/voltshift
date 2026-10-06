#ifndef VS_LED_H
#define VS_LED_H

#include <stdint.h>

#define VS_LED_MODE_OFF 0
#define VS_LED_MODE_ON 1
#define VS_LED_MODE_SLOW 2
#define VS_LED_MODE_FAST 3

void vs_led_init(void);
void vs_led_set_mode(uint8_t mode);
void vs_led_tick(uint32_t now);

#endif
