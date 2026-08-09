#ifndef EARTHBOUND_STATE_H
#define EARTHBOUND_STATE_H

#include <stdint.h>

enum {
    EB_KEY_B = 1u << 0,
    EB_KEY_Y = 1u << 1,
    EB_KEY_SELECT = 1u << 2,
    EB_KEY_START = 1u << 3,
    EB_KEY_UP = 1u << 4,
    EB_KEY_DOWN = 1u << 5,
    EB_KEY_LEFT = 1u << 6,
    EB_KEY_RIGHT = 1u << 7,
    EB_KEY_A = 1u << 8,
    EB_KEY_X = 1u << 9,
    EB_KEY_L = 1u << 10,
    EB_KEY_R = 1u << 11
};

typedef enum {
    EB_PARAM_LAYER,
    EB_PARAM_SPEED,
    EB_PARAM_AMPLITUDE,
    EB_PARAM_FREQUENCY,
    EB_PARAM_COMPRESSION,
    EB_PARAM_COUNT
} EbParameter;

typedef struct {
    uint16_t layer[2];
    int16_t speed[2];
    int16_t amplitude[2];
    int16_t frequency[2];
    int16_t compression[2];
    uint16_t random_state;
    uint8_t selected_layer;
    uint8_t selected_parameter;
    uint8_t debug_visible;
    uint8_t paused;
    uint8_t layers_dirty;
} EbState;

void eb_state_init(EbState *state);
void eb_state_press(EbState *state, uint16_t keys);

#endif
