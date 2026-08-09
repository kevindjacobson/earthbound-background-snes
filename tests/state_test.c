#include <assert.h>
#include <stdio.h>

#include "state.h"

static void test_defaults(void)
{
    EbState state;
    eb_state_init(&state);
    assert(state.layer[0] == 50);
    assert(state.layer[1] == 300);
    assert(state.selected_layer == 0);
    assert(state.selected_parameter == EB_PARAM_LAYER);
    assert(!state.debug_visible);
    assert(!state.paused);
}

static void test_layer_selection_and_wrap(void)
{
    EbState state;
    eb_state_init(&state);

    eb_state_press(&state, EB_KEY_R | EB_KEY_A);
    assert(state.selected_layer == 1);
    assert(state.layer[1] == 301);

    state.layer[1] = 326;
    eb_state_press(&state, EB_KEY_A);
    assert(state.layer[1] == 0);
    eb_state_press(&state, EB_KEY_B);
    assert(state.layer[1] == 326);

    eb_state_press(&state, EB_KEY_L);
    assert(state.selected_layer == 0);
}

static void test_debug_parameter_editing(void)
{
    EbState state;
    eb_state_init(&state);
    eb_state_press(&state, EB_KEY_START);
    assert(state.debug_visible);

    eb_state_press(&state, EB_KEY_SELECT);
    assert(state.selected_parameter == EB_PARAM_SPEED);
    eb_state_press(&state, EB_KEY_A);
    assert(state.speed[0] == 1);
    eb_state_press(&state, EB_KEY_B);
    assert(state.speed[0] == 0);

    eb_state_press(&state, EB_KEY_Y);
    assert(state.paused);
}

static void test_random_pair_is_deterministic_and_in_range(void)
{
    EbState first;
    EbState second;
    eb_state_init(&first);
    eb_state_init(&second);
    first.layers_dirty = 0;
    second.layers_dirty = 0;
    eb_state_press(&first, EB_KEY_X);
    eb_state_press(&second, EB_KEY_X);
    assert(first.layer[0] == second.layer[0]);
    assert(first.layer[1] == second.layer[1]);
    assert(first.layer[0] < 327);
    assert(first.layer[1] < 327);
    assert(first.layer[0] != first.layer[1]);
    assert(first.layers_dirty);
}

int main(void)
{
    test_defaults();
    test_layer_selection_and_wrap();
    test_debug_parameter_editing();
    test_random_pair_is_deterministic_and_in_range();
    puts("state tests passed");
    return 0;
}
