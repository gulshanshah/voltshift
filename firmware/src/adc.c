#include "adc.h"
#include "config.h"

#include <avr/io.h>

void vs_adc_init(void)
{
    ADMUX = _BV(REFS0);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & _BV(ADSC)) {
    }
    (void)ADC;
}

uint16_t vs_adc_read(uint8_t channel)
{
    uint32_t sum = 0;
    uint8_t i;

    ADMUX = _BV(REFS0) | (channel & 0x0FU);

    for (i = 0; i < VS_SAMPLE_COUNT; i++) {
        ADCSRA |= _BV(ADSC);
        while (ADCSRA & _BV(ADSC)) {
        }
        sum += ADC;
    }

    return (uint16_t)((sum + (VS_SAMPLE_COUNT / 2)) / VS_SAMPLE_COUNT);
}

uint16_t vs_adc_to_dv(uint8_t channel, uint16_t counts)
{
    static const int16_t offset[VS_CHANNEL_COUNT] = {
        VS_CAL_OFFSET_R,
        VS_CAL_OFFSET_Y,
        VS_CAL_OFFSET_B,
    };
    static const uint16_t span[VS_CHANNEL_COUNT] = {
        VS_CAL_SPAN_R_PER_100V,
        VS_CAL_SPAN_Y_PER_100V,
        VS_CAL_SPAN_B_PER_100V,
    };
    int32_t v;

    if (channel >= VS_CHANNEL_COUNT) {
        return 0;
    }

    v = (int32_t)counts - offset[channel];
    if (v <= 0) {
        return 0;
    }

    return (uint16_t)((v * 1000L) / span[channel]);
}
