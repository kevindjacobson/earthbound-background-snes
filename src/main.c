#include <snes.h>

#include "generated_assets.h"
#include "state.h"

#define BG_LAYER_1 0
#define BG_LAYER_2 1
#define BG_DEBUG 0
#define BG1_GFX 0x0000
#define BG2_GFX 0x2000
#define BG1_MAP 0x7000
#define BG2_MAP 0x7400

static EbState state;
static u16 map_buffer[1024];
static u16 palette_buffer[16];
static u16 frame_number;

static u16 translate_keys(u16 keys)
{
    u16 result = 0;
    if (keys & KEY_B) result |= EB_KEY_B;
    if (keys & KEY_Y) result |= EB_KEY_Y;
    if (keys & KEY_SELECT) result |= EB_KEY_SELECT;
    if (keys & KEY_START) result |= EB_KEY_START;
    if (keys & KEY_UP) result |= EB_KEY_UP;
    if (keys & KEY_DOWN) result |= EB_KEY_DOWN;
    if (keys & KEY_LEFT) result |= EB_KEY_LEFT;
    if (keys & KEY_RIGHT) result |= EB_KEY_RIGHT;
    if (keys & KEY_A) result |= EB_KEY_A;
    if (keys & KEY_X) result |= EB_KEY_X;
    if (keys & KEY_L) result |= EB_KEY_L;
    if (keys & KEY_R) result |= EB_KEY_R;
    return result;
}

static void load_map(u8 graphics, u8 palette, u16 destination)
{
    u8 *source = eb_map_source(graphics);
    u16 index;
    for (index = 0; index < 1024; ++index) {
        u16 offset = index << 1;
        u16 word = source[offset] | ((u16)source[offset + 1] << 8);
        map_buffer[index] = (word & 0xe3ff) | ((u16)palette << 10);
    }
    dmaCopyVram((u8 *)map_buffer, destination, sizeof(map_buffer));
}

static u8 cycle_source(const EbLayerSpec *layer, u8 index, u16 frame)
{
    u16 interval;
    u16 position;
    u8 start;
    u8 end;
    u8 length;
    s16 source;

    if (!layer->cycle_type || !layer->cycle_speed) return index;
    interval = (layer->cycle_speed + 1) >> 1;
    position = (frame + 1) / interval;
    if (position) --position;
    if (!position) return index;

    start = layer->cycle_1_start;
    end = layer->cycle_1_end;
    if ((layer->cycle_type == 1 || layer->cycle_type == 2) && index >= start && index <= end) {
        length = end - start + 1;
        source = index - (position % length);
        if (source < start) source += length;
        return (u8)source;
    }
    if (layer->cycle_type == 2) {
        start = layer->cycle_2_start;
        end = layer->cycle_2_end;
        if (index >= start && index <= end) {
            length = end - start + 1;
            source = index - (position % length);
            if (source < start) source += length;
            return (u8)source;
        }
    }
    if (layer->cycle_type == 3 && index >= start && index <= end) {
        u8 difference;
        length = end - start + 1;
        source = index + (position % (length << 1));
        if (source > end) {
            difference = source - end - 1;
            source = end - difference;
            if (source < start) {
                difference = start - source - 1;
                source = start + difference;
            }
        }
        return (u8)source;
    }
    return index;
}

static void load_palette(u8 slot, u16 layer_id, u16 frame)
{
    const EbLayerSpec *layer = &eb_layers[layer_id];
    const u16 *source = eb_palettes[layer->palette];
    u8 count = layer->bits_per_pixel == 2 ? 4 : 16;
    u8 index;

    for (index = 0; index < count; ++index) palette_buffer[index] = source[cycle_source(layer, index, frame)];
    for (; index < 16; ++index) palette_buffer[index] = 0;
    dmaCopyCGram((u8 *)palette_buffer, slot << 4, 32);
}

static void load_layers(void)
{
    const EbLayerSpec *first = &eb_layers[state.layer[0]];
    const EbLayerSpec *second = &eb_layers[state.layer[1]];

    setMode(BG_MODE1, 0);
    bgSetGfxPtr(BG_LAYER_1, BG1_GFX);
    bgSetGfxPtr(BG_LAYER_2, BG2_GFX);
    bgSetMapPtr(BG_LAYER_1, BG1_MAP, SC_32x32);
    bgSetMapPtr(BG_LAYER_2, BG2_MAP, SC_32x32);

    dmaCopyVram(eb_graphics_source(first->graphics), BG1_GFX,
                eb_graphics_lengths[first->graphics]);
    dmaCopyVram(eb_graphics_source(second->graphics), BG2_GFX,
                eb_graphics_lengths[second->graphics]);
    load_map(first->graphics, 0, BG1_MAP);
    load_map(second->graphics, 1, BG2_MAP);
    load_palette(0, state.layer[0], frame_number);
    load_palette(1, state.layer[1], frame_number);

    bgSetEnable(BG_LAYER_1);
    bgSetEnable(BG_LAYER_2);
    bgSetDisable(2);
    bgSetDisable(3);

    REG_TM = 0x01;
    REG_TS = 0x02;
    REG_CGWSEL = 0x02;
    REG_CGADSUB = 0x41;
}

static void draw_parameter_name(u8 parameter)
{
    if (parameter == EB_PARAM_LAYER) consoleDrawText(7, 15, "LAYER      ");
    else if (parameter == EB_PARAM_SPEED) consoleDrawText(7, 15, "SPEED      ");
    else if (parameter == EB_PARAM_AMPLITUDE) consoleDrawText(7, 15, "AMPLITUDE  ");
    else if (parameter == EB_PARAM_FREQUENCY) consoleDrawText(7, 15, "FREQUENCY  ");
    else if (parameter == EB_PARAM_COMPRESSION) consoleDrawText(7, 15, "COMPRESSION");
    else consoleDrawText(7, 15, "?          ");
    consoleDrawText(20, 15, "[%u]", (u16)parameter);
}

static void draw_debug(void)
{
    const EbLayerSpec *first = &eb_layers[state.layer[0]];
    const EbLayerSpec *second = &eb_layers[state.layer[1]];
    const EbEffectSpec *effect_1 = &eb_effects[first->effect];
    const EbEffectSpec *effect_2 = &eb_effects[second->effect];

    consoleDrawText(1, 1, "EARTHBOUND BACKGROUND LAB");
    consoleDrawText(1, 3, state.selected_layer == 0 ? ">" : " ");
    consoleDrawText(3, 3, "LAYER 1  ID %03u", state.layer[0]);
    consoleDrawText(1, 4, "  GFX %03u PAL %03u BPP %u", (u16)first->graphics, (u16)first->palette, (u16)first->bits_per_pixel);
    consoleDrawText(1, 5, "  FX %03u TYPE %u", (u16)first->effect, (u16)effect_1->type);
    consoleDrawText(1, 6, "  SPD %d AMP %d", effect_1->speed + state.speed[0], effect_1->amplitude + state.amplitude[0]);
    consoleDrawText(1, 7, "  FREQ %d COMP %d", effect_1->frequency + state.frequency[0], effect_1->compression + state.compression[0]);
    consoleDrawText(1, 9, state.selected_layer == 1 ? ">" : " ");
    consoleDrawText(3, 9, "LAYER 2  ID %03u", state.layer[1]);
    consoleDrawText(1, 10, "  GFX %03u PAL %03u BPP %u", (u16)second->graphics, (u16)second->palette, (u16)second->bits_per_pixel);
    consoleDrawText(1, 11, "  FX %03u TYPE %u", (u16)second->effect, (u16)effect_2->type);
    consoleDrawText(1, 12, "  SPD %d AMP %d", effect_2->speed + state.speed[1], effect_2->amplitude + state.amplitude[1]);
    consoleDrawText(1, 13, "  FREQ %d COMP %d", effect_2->frequency + state.frequency[1], effect_2->compression + state.compression[1]);
    consoleDrawText(1, 15, "EDIT:");
    draw_parameter_name(state.selected_parameter);
    consoleDrawText(1, 16, "ANIMATION: %s", state.paused ? "PAUSED " : "RUNNING");
    consoleDrawText(1, 18, "L/R       SELECT LAYER");
    consoleDrawText(1, 19, "SELECT    SELECT PARAMETER");
    consoleDrawText(1, 20, "D-PAD A/B CHANGE VALUE");
    consoleDrawText(1, 21, "X         RANDOM PAIR");
    consoleDrawText(1, 22, "Y         PAUSE");
    consoleDrawText(1, 23, "START     BACKGROUND");
}

static void show_debug(void)
{
    consoleInitDefaultText(0);
    bgSetGfxPtr(BG_DEBUG, 0x3000);
    bgSetMapPtr(BG_DEBUG, 0x6800, SC_32x32);
    setMode(BG_MODE1, 0);
    bgSetEnable(BG_DEBUG);
    bgSetDisable(1);
    bgSetDisable(2);
    bgSetDisable(3);
    REG_TM = 0x01;
    REG_TS = 0;
    REG_CGWSEL = 0;
    REG_CGADSUB = 0;
    draw_debug();
}

static s16 triangle_wave(u16 phase)
{
    u8 x = (u8)phase;
    if (x < 64) return x;
    if (x < 192) return 128 - x;
    return x - 256;
}

static void animate_layer(u8 screen, u16 layer_id, u8 slot)
{
    const EbLayerSpec *layer = &eb_layers[layer_id];
    const EbEffectSpec *effect = &eb_effects[layer->effect];
    s16 speed = effect->speed + state.speed[slot];
    s16 amplitude = (effect->amplitude + state.amplitude[slot]) >> 8;
    s16 frequency = (effect->frequency + state.frequency[slot]) >> 8;
    u16 x = (u16)((frame_number * speed) >> 2);
    u16 y = (u16)((triangle_wave(frame_number * frequency) * amplitude) >> 6);
    bgSetScroll(screen, x, y);
}

int main(void)
{
    u16 pressed;
    frame_number = 0;
    eb_state_init(&state);
    setScreenOff();
    load_layers();
    state.layers_dirty = 0;
    setScreenOn();

    while (1) {
        WaitForVBlank();
        pressed = translate_keys(padsDown(0));
        if (pressed) eb_state_press(&state, pressed);

        if (state.layers_dirty) {
            setScreenOff();
            if (state.debug_visible) show_debug();
            else load_layers();
            state.layers_dirty = 0;
            setScreenOn();
        } else if (state.debug_visible && pressed) {
            draw_debug();
        }

        if (!state.debug_visible && !state.paused) {
            ++frame_number;
            animate_layer(BG_LAYER_1, state.layer[0], 0);
            animate_layer(BG_LAYER_2, state.layer[1], 1);
            if ((frame_number & 7) == 0) {
                load_palette(0, state.layer[0], frame_number);
                load_palette(1, state.layer[1], frame_number);
            }
        }
    }
    return 0;
}
