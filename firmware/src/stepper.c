#include "stepper.h"
#include "config.h"

#include <avr/io.h>

static const uint8_t s_pattern[8] = { 0x01U, 0x03U, 0x02U, 0x06U, 0x04U, 0x0CU, 0x08U, 0x09U };

static int16_t s_abs_step;
static int16_t s_remaining;
static uint8_t s_target;
static uint8_t s_step_index;
static uint32_t s_next_ms;

static void coil_write(uint8_t index)
{
    uint8_t value = s_pattern[index & 0x07U];

#if VS_STEP_HOLD_POWER == 0
    if (s_remaining == 0) {
        value = 0;
    }
#endif

    VS_STEPPER_DDR |= VS_STEPPER_MASK;
    VS_STEPPER_PORT = (uint8_t)((VS_STEPPER_PORT & (uint8_t)~VS_STEPPER_MASK) | value);
}

void vs_stepper_init(void)
{
    s_target = VS_BOOT_POSITION;
    s_abs_step = (int16_t)(VS_BOOT_POSITION * VS_HALF_STEPS_PER_POS);
    s_remaining = 0;
    s_step_index = 0;
    s_next_ms = 0;

    VS_STEPPER_DDR |= VS_STEPPER_MASK;
    VS_STEPPER_PORT &= (uint8_t)~VS_STEPPER_MASK;
#if VS_STEP_HOLD_POWER
    coil_write(s_step_index);
#endif
}

void vs_stepper_goto(uint8_t position)
{
    int16_t target;

    if (position >= VS_SEL_POS_COUNT) {
        position = VS_SEL_POS_OFF;
    }

    target = (int16_t)(position * VS_HALF_STEPS_PER_POS);
    s_target = position;
    s_remaining = (int16_t)(target - s_abs_step);
}

void vs_stepper_tick(uint32_t now)
{
    int8_t dir;

    if (s_remaining == 0) {
#if VS_STEP_HOLD_POWER
        coil_write(s_step_index);
#endif
        return;
    }

    if ((int32_t)(now - s_next_ms) < 0) {
        return;
    }

    dir = s_remaining > 0 ? 1 : -1;
    s_step_index = (uint8_t)((s_step_index + dir + 8) & 0x07);
    s_abs_step = (int16_t)(s_abs_step + dir);
    s_remaining = (int16_t)(s_remaining - dir);
    coil_write(s_step_index);
    s_next_ms = now + VS_STEP_INTERVAL_MS;
}

uint8_t vs_stepper_busy(void)
{
    return s_remaining != 0;
}

uint8_t vs_stepper_position(void)
{
    return s_target;
}
