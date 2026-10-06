#include "led.h"
#include "config.h"

#include <avr/io.h>

static uint8_t s_mode = VS_LED_MODE_OFF;
static uint8_t s_level;
static uint32_t s_next_ms;

static void led_write(uint8_t on)
{
    s_level = on;

#if VS_LED_ACTIVE_LOW
    if (on) {
        VS_LED_PORT &= (uint8_t)~_BV(VS_LED_BIT);
    } else {
        VS_LED_PORT |= _BV(VS_LED_BIT);
    }
#else
    if (on) {
        VS_LED_PORT |= _BV(VS_LED_BIT);
    } else {
        VS_LED_PORT &= (uint8_t)~_BV(VS_LED_BIT);
    }
#endif
}

void vs_led_init(void)
{
    VS_LED_DDR |= _BV(VS_LED_BIT);
    s_mode = VS_LED_MODE_OFF;
    s_next_ms = 0;
    led_write(0);
}

void vs_led_set_mode(uint8_t mode)
{
    if (mode == s_mode) {
        return;
    }
    s_mode = mode;

    if (mode == VS_LED_MODE_OFF) {
        led_write(0);
    } else if (mode == VS_LED_MODE_ON) {
        led_write(1);
    }
}

void vs_led_tick(uint32_t now)
{
    uint32_t interval;

    if (s_mode == VS_LED_MODE_OFF || s_mode == VS_LED_MODE_ON) {
        return;
    }

    interval = s_mode == VS_LED_MODE_SLOW ? 500UL : 100UL;

    if ((int32_t)(now - s_next_ms) < 0) {
        return;
    }

    led_write(s_level ? 0 : 1);
    s_next_ms = now + interval;
}
