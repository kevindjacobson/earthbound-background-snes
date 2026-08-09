#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "distortion.h"

static EbDistortionParams effect(uint8_t type, int16_t frequency, int16_t amplitude,
                                 int16_t compression, int8_t speed)
{
    EbDistortionParams value = {0};
    value.type = type;
    value.frequency = frequency;
    value.amplitude = amplitude;
    value.compression = compression;
    value.speed = speed;
    return value;
}

static void test_horizontal_smooth_uses_each_scanline(void)
{
    uint16_t lines[EB_VISIBLE_LINES];
    EbDistortionParams value = effect(1, 0x0400, 0x2000, 0, 0);

    eb_distortion_build(lines, &value, 0, 0);

    assert(lines[0] == 0);
    assert(lines[1] == 1);
    assert(lines[2] == 3);
    assert(lines[16] == 15);
}

static void test_horizontal_interlaced_alternates_direction(void)
{
    uint16_t lines[EB_VISIBLE_LINES];
    EbDistortionParams value = effect(2, 0x4000, 0x2000, 0, 0);

    eb_distortion_build(lines, &value, 0, 0);

    assert(lines[0] == 0);
    assert((int16_t)lines[1] == -15);
    assert(lines[2] == 0);
    assert(lines[3] == 16);
}

static void test_vertical_smooth_applies_compression(void)
{
    uint16_t lines[EB_VISIBLE_LINES];
    EbDistortionParams value = effect(3, 0, 0, 0x0100, 0);

    eb_distortion_build(lines, &value, 0, 7);

    assert(lines[0] == 8);
    assert(lines[1] == 9);
    assert(lines[2] == 10);
    assert(lines[223] == 231);
}

static void test_frame_acceleration_matches_runtime_updates(void)
{
    uint16_t lines[EB_VISIBLE_LINES];
    EbDistortionParams value = effect(1, 0, 0x2000, 0, 2);
    value.amplitude_acceleration = 0x0100;

    eb_distortion_build(lines, &value, 32, 3);

    assert(lines[0] == 34);
    assert(lines[100] == 34);
}

static void test_hdma_descriptor_covers_the_visible_frame(void)
{
    uint8_t descriptor[EB_HDMA_DESCRIPTOR_SIZE];

    eb_hdma_descriptor(descriptor, 0x3c46);

    assert(descriptor[0] == 0xe4);
    assert(descriptor[1] == 0x46);
    assert(descriptor[2] == 0x3c);
    assert(descriptor[3] == 0xfc);
    assert(descriptor[4] == 0x0e);
    assert(descriptor[5] == 0x3d);
    assert(descriptor[6] == 0);
}

int main(void)
{
    test_horizontal_smooth_uses_each_scanline();
    test_horizontal_interlaced_alternates_direction();
    test_vertical_smooth_applies_compression();
    test_frame_acceleration_matches_runtime_updates();
    test_hdma_descriptor_covers_the_visible_frame();
    puts("distortion tests passed");
    return 0;
}
