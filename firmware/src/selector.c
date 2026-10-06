#include "selector.h"
#include "adc.h"
#include "config.h"

typedef struct {
    uint16_t dv;
    int32_t ema_dv;
    uint16_t min_dv;
    uint16_t max_dv;
    uint8_t valid;
} vs_channel_t;

static vs_channel_t s_ch[VS_CHANNEL_COUNT];
static uint8_t s_position = VS_SEL_POS_OFF;
static int8_t s_pending = -1;
static uint8_t s_pending_count;

static int32_t iabs32(int32_t v)
{
    return v < 0 ? -v : v;
}

static void window_reset(uint8_t i)
{
    s_ch[i].min_dv = 0xFFFFU;
    s_ch[i].max_dv = 0;
}

void vs_selector_init(void)
{
    uint8_t i;

    for (i = 0; i < VS_CHANNEL_COUNT; i++) {
        s_ch[i].dv = 0;
        s_ch[i].ema_dv = 0;
        s_ch[i].valid = 0;
        window_reset(i);
    }
    s_position = VS_SEL_POS_OFF;
    s_pending = -1;
    s_pending_count = 0;
}

void vs_selector_sample(void)
{
    uint8_t i;

    for (i = 0; i < VS_CHANNEL_COUNT; i++) {
        uint16_t dv = vs_adc_to_dv(i, vs_adc_read(i));

        s_ch[i].dv = dv;
        s_ch[i].ema_dv += ((int32_t)dv - s_ch[i].ema_dv) / VS_EMA_DIV;
        if (dv < s_ch[i].min_dv) {
            s_ch[i].min_dv = dv;
        }
        if (dv > s_ch[i].max_dv) {
            s_ch[i].max_dv = dv;
        }
    }
}

void vs_selector_evaluate(uint8_t startup)
{
    int32_t best_err = 0x7FFFFFFFL;
    int8_t best = -1;
    uint8_t current_valid;
    uint8_t i;

    for (i = 0; i < VS_CHANNEL_COUNT; i++) {
        uint16_t ripple = (uint16_t)(s_ch[i].max_dv - s_ch[i].min_dv);
        uint16_t dv = (uint16_t)s_ch[i].ema_dv;
        int32_t err;

        s_ch[i].valid = (dv >= VS_MIN_DV && dv <= VS_MAX_DV && ripple <= VS_MAX_RIPPLE_DV) ? 1 : 0;
        window_reset(i);

        if (s_ch[i].valid) {
            err = iabs32((int32_t)dv - VS_TARGET_DV);
            if (err < best_err) {
                best_err = err;
                best = (int8_t)i;
            }
        }
    }

    if (startup) {
        s_position = VS_SEL_POS_OFF;
        s_pending = -1;
        s_pending_count = 0;
        return;
    }

    current_valid = (s_position < VS_SEL_POS_COUNT - 1) ? s_ch[s_position].valid : 0;

    if (!current_valid) {
        s_position = (best >= 0) ? (uint8_t)best : VS_SEL_POS_OFF;
        s_pending = -1;
        s_pending_count = 0;
        return;
    }

    if (best >= 0 && best != (int8_t)s_position) {
        int32_t current_err = iabs32((int32_t)s_ch[s_position].dv - VS_TARGET_DV);

        if (current_err - best_err > VS_SWITCH_MARGIN_DV) {
            if (s_pending == best) {
                s_pending_count++;
            } else {
                s_pending = best;
                s_pending_count = 1;
            }
            if (s_pending_count >= VS_CONFIRM_EVALS) {
                s_position = (uint8_t)best;
                s_pending = -1;
                s_pending_count = 0;
            }
        } else {
            s_pending = -1;
            s_pending_count = 0;
        }
    } else {
        s_pending = -1;
        s_pending_count = 0;
    }
}

uint8_t vs_selector_position(void)
{
    return s_position;
}

uint8_t vs_selector_valid(uint8_t channel)
{
    if (channel >= VS_CHANNEL_COUNT) {
        return 0;
    }
    return s_ch[channel].valid;
}

uint16_t vs_selector_dv(uint8_t channel)
{
    if (channel >= VS_CHANNEL_COUNT) {
        return 0;
    }
    return (uint16_t)s_ch[channel].ema_dv;
}

uint8_t vs_selector_mask(void)
{
    uint8_t mask = 0;
    uint8_t i;

    for (i = 0; i < VS_CHANNEL_COUNT; i++) {
        if (s_ch[i].valid) {
            mask |= (uint8_t)(1U << i);
        }
    }
    return mask;
}
