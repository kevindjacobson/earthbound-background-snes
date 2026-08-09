#include "distortion.h"

static const uint8_t sine_quarter[65] = {
    0, 3, 6, 9, 12, 15, 18, 21, 24, 28, 31, 34, 37, 40, 43, 46,
    48, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88,
    90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 109, 111, 112, 114,
    115, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 126, 126,
    127, 127, 127, 127, 127, 127
};

static int16_t sine(uint8_t phase)
{
    if (phase <= 64) return sine_quarter[phase];
    if (phase <= 128) return sine_quarter[128 - phase];
    if (phase <= 192) return -(int16_t)sine_quarter[phase - 128];
    return -(int16_t)sine_quarter[256 - phase];
}

#ifdef __65816__
extern void eb_distortion_build_snes(uint16_t *output, uint16_t type,
                                     uint16_t frequency, uint16_t amplitude,
                                     uint16_t compression, uint16_t phase,
                                     uint16_t base_scroll);
#endif

static int16_t product_middle(uint8_t amplitude, int16_t wave)
{
    int32_t product = (int32_t)amplitude * wave;
    if (product >= 0) return (int16_t)(product / 256);
    return (int16_t)-((-product + 255) / 256);
}

static uint16_t accelerated(int16_t initial, int16_t acceleration, uint16_t frame)
{
    return (uint16_t)((uint16_t)initial + (uint16_t)acceleration * frame);
}

void eb_distortion_build(uint16_t output[EB_VISIBLE_LINES],
                         const EbDistortionParams *effect,
                         uint16_t frame,
                         int16_t base_scroll)
{
    uint16_t frequency = accelerated(effect->frequency,
                                     effect->frequency_acceleration, frame);
    uint16_t amplitude = accelerated(effect->amplitude,
                                     effect->amplitude_acceleration, frame);
    uint16_t compression = accelerated(effect->compression,
                                       effect->compression_acceleration, frame);
    uint16_t phase = (uint16_t)((uint8_t)(effect->speed * frame)) << 8;
    uint16_t vertical = (uint16_t)((uint8_t)base_scroll) << 8;
    uint8_t amplitude_byte = (uint8_t)(amplitude >> 8);
    uint16_t line;

#ifdef __65816__
    eb_distortion_build_snes(output, effect->type, frequency, amplitude_byte,
                             compression, (uint8_t)(effect->speed * frame),
                             (uint16_t)base_scroll);
    return;
#endif

    for (line = 0; line < EB_VISIBLE_LINES; ++line) {
        int16_t offset = product_middle(amplitude_byte, sine((uint8_t)(phase >> 8)));

        if (effect->type == 3) {
            vertical += compression;
            output[line] = (uint16_t)(((vertical >> 8) & 0xff) + offset);
        } else if (effect->type == 2 && (line & 1)) {
            output[line] = (uint16_t)(base_scroll - offset);
        } else {
            output[line] = (uint16_t)(base_scroll + offset);
        }
        phase += frequency;
    }
}

void eb_hdma_descriptor(uint8_t output[EB_HDMA_DESCRIPTOR_SIZE], uint16_t address)
{
    output[0] = 0xe4;
    output[1] = (uint8_t)address;
    output[2] = (uint8_t)(address >> 8);
    address += 200;
    output[3] = 0xfc;
    output[4] = (uint8_t)address;
    output[5] = (uint8_t)(address >> 8);
    output[6] = 0;
}
