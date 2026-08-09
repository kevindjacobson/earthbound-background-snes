# EarthBound Background Lab for SNES

A controller-driven SNES ROM for exploring EarthBound battle-background layers. It carries the complete 327-layer catalog and native tile, arrangement, palette, and effect data in a 1 MiB LoROM image.

The renderer follows the original game's hardware design: two Mode 1 backgrounds, SNES color math, palette cycling, and per-scanline HDMA writes to each layer's horizontal or vertical scroll register. It supports smooth horizontal, interlaced horizontal, and smooth vertical distortion, including frequency, amplitude, compression, speed, and their acceleration fields.

The hot scanline loop is 65816 assembly and uses the PPU's Mode 7 multiplier. Two WRAM buffers per layer keep HDMA away from a table while it is being rebuilt. A small opaque BG3 fallback preserves half-color blending when the second layer uses palette index zero.

## Controls

- L / R: select layer 1 or layer 2
- D-pad or A / B: raise or lower the selected value; Up and Down move by 10
- X: choose a random pair
- Y: pause or resume animation
- Start: open or close the debug screen
- Select: choose the field edited in the debug screen

The debug screen reports each layer ID, graphics bank, palette, effect, effect type, speed, amplitude, and frequency. Speed, amplitude, frequency, and compression can be adjusted independently for either layer.

## Build

The project is pinned to [PVSnesLib 4.6.0](https://github.com/alekmaul/pvsneslib/releases/tag/4.6.0).

```sh
export PVSNESLIB_HOME=/path/to/pvsneslib
make generate
make test
make
```

The ROM is written to `earthbound_background_lab.sfc`.

`make generate` rebuilds the SNES assets and C tables from the two JSON files in `data/`. Two-bit graphics banks are expanded to four-bit planar tiles so both backgrounds can share Mode 1.

## Verification

Host tests cover controller state, wraparound, random selection, asset conversion, distortion math, and HDMA descriptors. The ROM is also built in GitHub Actions. For local emulator checks:

```sh
/path/to/Mesen --testrunner --timeout=10 \
  tests/mesen-smoke.lua earthbound_background_lab.sfc

/path/to/Mesen --testrunner --timeout=10 \
  tests/mesen-animation.lua earthbound_background_lab.sfc
```

The smoke script exercises Start, Select, R, and A and checks both the debug screen and a rendered background. The animation script checks that scanline tables keep advancing under emulation instead of merely producing one valid frame.

Real-hardware behavior has not yet been checked on a SNES or flash cartridge.

## Examples

The [ten-second emulator capture](examples/earthbound-background-lab-demo.mp4) changes the pair, opens the debug screen, selects layer 2, raises its speed, returns to the background, and changes the pair again.

![Default layer pair](examples/pair-50-300.png)

![Random layer pair](examples/random-pair.png)

![Debug screen with layer 2 selected](examples/debug-layer-2.png)

## Provenance

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for the extracted background-data source and license record. Game names and original game assets belong to their respective owners; this is an independent technical project.

The HDMA implementation was checked against Herringway's EarthBound disassembly, particularly [`PREPARE_BG_OFFSET_TABLES`](https://github.com/Herringway/ebsrc/blob/0197d6c13ef11ad3280e9388e08a646ab1030d15/src/misc/battlebgs/prepare_bg_offset_tables.asm) and [`DO_BATTLEBG_DMA`](https://github.com/Herringway/ebsrc/blob/0197d6c13ef11ad3280e9388e08a646ab1030d15/src/misc/battlebgs/do_battlebg_dma.asm). The table layout follows the [SNESdev HDMA examples](https://snes.nesdev.org/wiki/HDMA_examples).
