#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#include "adc.h"
#include "config.h"
#include "led.h"
#include "selector.h"
#include "stepper.h"

static volatile uint32_t s_ms;

ISR(TIMER0_COMPA_vect)
{
    s_ms++;
}

static uint32_t millis(void)
{
    uint32_t value;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = s_ms;
    }
    return value;
}

static void timer_init(void)
{
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS01) | _BV(CS00);
    OCR0A = 249;
    TIMSK0 = _BV(OCIE0A);
}

static void select_mode(uint8_t startup)
{
    if (startup) {
        vs_led_set_mode(VS_LED_MODE_SLOW);
    } else if (vs_stepper_busy() || vs_selector_position() == VS_SEL_POS_OFF) {
        vs_led_set_mode(VS_LED_MODE_FAST);
    } else {
        vs_led_set_mode(VS_LED_MODE_ON);
    }
}

int main(void)
{
    uint32_t now;
    uint32_t last_sample = 0;
    uint32_t last_eval = 0;

    vs_adc_init();
    vs_selector_init();
    vs_stepper_init();
    vs_led_init();
    timer_init();
    wdt_enable(WDTO_1S);
    sei();

    for (;;) {
        now = millis();

        if ((now - last_sample) >= VS_SAMPLE_MS) {
            last_sample = now;
            vs_selector_sample();
        }

        if ((now - last_eval) >= VS_EVAL_MS) {
            uint8_t startup = now < VS_STARTUP_MS;

            last_eval = now;
            vs_selector_evaluate(startup);
            vs_stepper_goto(vs_selector_position());
            select_mode(startup);
        }

        vs_stepper_tick(now);
        vs_led_tick(now);
        wdt_reset();
    }
}
