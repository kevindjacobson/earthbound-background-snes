#include "state.h"

#define EB_LAYER_COUNT 327

static uint16_t wrap_layer(int value)
{
    while (value < 0) value += EB_LAYER_COUNT;
    while (value >= EB_LAYER_COUNT) value -= EB_LAYER_COUNT;
    return (uint16_t)value;
}

static uint16_t next_random(uint16_t *state)
{
    uint16_t x = *state;
    x ^= (uint16_t)(x << 7);
    x ^= (uint16_t)(x >> 9);
    x ^= (uint16_t)(x << 8);
    *state = x;
    return x;
}

static int key_delta(uint16_t keys)
{
    int delta = 0;
    if (keys & (EB_KEY_A | EB_KEY_RIGHT)) delta += 1;
    if (keys & (EB_KEY_B | EB_KEY_LEFT)) delta -= 1;
    if (keys & EB_KEY_UP) delta += 10;
    if (keys & EB_KEY_DOWN) delta -= 10;
    return delta;
}

void eb_state_init(EbState *state)
{
    unsigned i;
    state->layer[0] = 50;
    state->layer[1] = 300;
    for (i = 0; i < 2; ++i) {
        state->speed[i] = 0;
        state->amplitude[i] = 0;
        state->frequency[i] = 0;
        state->compression[i] = 0;
    }
    state->random_state = 0x4f31;
    state->selected_layer = 0;
    state->selected_parameter = EB_PARAM_LAYER;
    state->debug_visible = 0;
    state->paused = 0;
    state->layers_dirty = 1;
}

void eb_state_press(EbState *state, uint16_t keys)
{
    uint8_t selected;
    int delta;

    if (keys & EB_KEY_L) state->selected_layer = 0;
    if (keys & EB_KEY_R) state->selected_layer = 1;
    selected = state->selected_layer;

    if (keys & EB_KEY_START) {
        state->debug_visible ^= 1;
        state->layers_dirty = 1;
    }
    if (keys & EB_KEY_Y) state->paused ^= 1;
    if (keys & EB_KEY_SELECT) {
        state->selected_parameter = (uint8_t)((state->selected_parameter + 1) % EB_PARAM_COUNT);
    }
    if (keys & EB_KEY_X) {
        state->layer[0] = (uint16_t)(next_random(&state->random_state) % EB_LAYER_COUNT);
        do {
            state->layer[1] = (uint16_t)(next_random(&state->random_state) % EB_LAYER_COUNT);
        } while (state->layer[0] == state->layer[1]);
        state->layers_dirty = 1;
    }

    delta = key_delta(keys);
    if (!delta) return;

    switch ((EbParameter)state->selected_parameter) {
    case EB_PARAM_LAYER:
        state->layer[selected] = wrap_layer((int)state->layer[selected] + delta);
        state->layers_dirty = 1;
        break;
    case EB_PARAM_SPEED:
        state->speed[selected] += delta;
        break;
    case EB_PARAM_AMPLITUDE:
        state->amplitude[selected] += delta;
        break;
    case EB_PARAM_FREQUENCY:
        state->frequency[selected] += delta;
        break;
    case EB_PARAM_COMPRESSION:
        state->compression[selected] += delta;
        break;
    default:
        break;
    }
}
