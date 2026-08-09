#ifndef EARTHBOUND_DISTORTION_H
#define EARTHBOUND_DISTORTION_H

#include <stdint.h>

#define EB_VISIBLE_LINES 224
#define EB_HDMA_DESCRIPTOR_SIZE 7

typedef struct {
    uint8_t type;
    int16_t frequency;
    int16_t amplitude;
    int16_t compression;
    int8_t speed;
    int16_t frequency_acceleration;
    int16_t amplitude_acceleration;
    int16_t compression_acceleration;
} EbDistortionParams;

void eb_distortion_build(uint16_t output[EB_VISIBLE_LINES],
                         const EbDistortionParams *effect,
                         uint16_t frame,
                         int16_t base_scroll);
void eb_hdma_descriptor(uint8_t output[EB_HDMA_DESCRIPTOR_SIZE], uint16_t address);

#endif
