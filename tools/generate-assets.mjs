import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");

export function expandTo4Bpp(bytes, bitsPerPixel) {
  if (bitsPerPixel === 4) return Buffer.from(bytes);
  if (bitsPerPixel !== 2 || bytes.length % 16 !== 0) {
    throw new RangeError(`unsupported graphics data: ${bitsPerPixel}bpp, ${bytes.length} bytes`);
  }
  const result = Buffer.alloc(bytes.length * 2);
  for (let source = 0, target = 0; source < bytes.length; source += 16, target += 32) {
    bytes.copy(result, target, source, source + 16);
  }
  return result;
}

export function normalizeMap(bytes) {
  if (bytes.length !== 2048) throw new RangeError(`expected a 32x32 map, got ${bytes.length} bytes`);
  const result = Buffer.from(bytes);
  for (let offset = 0; offset < result.length; offset += 2) {
    const word = result.readUInt16LE(offset) & ~0x1c00;
    result.writeUInt16LE(word, offset);
  }
  return result;
}

function cValues(values, columns = 12) {
  const rows = [];
  for (let i = 0; i < values.length; i += columns) rows.push(`    ${values.slice(i, i + columns).join(", ")}`);
  return rows.join(",\n");
}

export function generate({ layersPath, nativePath, outputDirectory }) {
  const catalog = JSON.parse(fs.readFileSync(layersPath, "utf8"));
  const native = JSON.parse(fs.readFileSync(nativePath, "utf8"));
  const graphics = Buffer.from(native.graphics.bytesBase64, "base64");
  const maps = Buffer.from(native.arrangements.wordsBase64, "base64");
  const assetsDirectory = path.join(outputDirectory, "assets");
  fs.mkdirSync(assetsDirectory, { recursive: true });

  const assembly = [`.include "hdr.asm"`, ""];
  const declarations = [
    "#ifndef EARTHBOUND_GENERATED_ASSETS_H",
    "#define EARTHBOUND_GENERATED_ASSETS_H",
    "",
    "#include <stdint.h>",
    "",
  ];
  const lengths = [];

  for (let index = 0; index < native.graphics.offsets.length; index += 1) {
    const name = String(index).padStart(3, "0");
    const offset = native.graphics.offsets[index];
    const length = native.graphics.lengths[index];
    const source = graphics.subarray(offset, offset + length);
    const converted = expandTo4Bpp(source, native.graphics.bitsPerPixel[index]);
    const mapOffset = index * native.arrangements.wordsPerBank * 2;
    const map = normalizeMap(maps.subarray(mapOffset, mapOffset + 2048));
    fs.writeFileSync(path.join(assetsDirectory, `gfx_${name}.bin`), converted);
    fs.writeFileSync(path.join(assetsDirectory, `map_${name}.bin`), map);
    lengths.push(converted.length);
    declarations.push(`extern uint8_t gfx_${name};`, `extern uint8_t map_${name};`);
    assembly.push(
      `.section ".earthbound_${name}" superfree`,
      `gfx_${name}:`,
      `.incbin "generated/assets/gfx_${name}.bin"`,
      `map_${name}:`,
      `.incbin "generated/assets/map_${name}.bin"`,
      ".ends",
      "",
    );
  }

  declarations.push(
    "",
    `#define EB_GRAPHICS_COUNT ${native.graphics.offsets.length}`,
    `#define EB_LAYER_COUNT ${catalog.layers.length}`,
    `#define EB_PALETTE_COUNT ${catalog.palettes.length}`,
    `#define EB_EFFECT_COUNT ${catalog.effects.length}`,
    "",
    "typedef struct {",
    "    uint8_t graphics;",
    "    uint8_t palette;",
    "    uint8_t bits_per_pixel;",
    "    uint8_t cycle_type;",
    "    uint8_t cycle_1_start;",
    "    uint8_t cycle_1_end;",
    "    uint8_t cycle_2_start;",
    "    uint8_t cycle_2_end;",
    "    uint8_t cycle_speed;",
    "    uint8_t effect;",
    "} EbLayerSpec;",
    "",
    "typedef struct {",
    "    uint8_t type;",
    "    int16_t frequency;",
    "    int16_t amplitude;",
    "    int16_t compression;",
    "    int16_t speed;",
    "} EbEffectSpec;",
    "",
    "extern const uint16_t eb_graphics_lengths[EB_GRAPHICS_COUNT];",
    "extern const EbLayerSpec eb_layers[EB_LAYER_COUNT];",
    "extern const EbEffectSpec eb_effects[EB_EFFECT_COUNT];",
    "extern const uint16_t eb_palettes[EB_PALETTE_COUNT][16];",
    "uint8_t *eb_graphics_source(uint8_t index);",
    "uint8_t *eb_map_source(uint8_t index);",
    "",
    "#endif",
    "",
  );

  const source = [
    '#include "generated_assets.h"',
    "",
    `const uint16_t eb_graphics_lengths[EB_GRAPHICS_COUNT] = {\n${cValues(lengths)}\n};`,
    "",
    "const EbLayerSpec eb_layers[EB_LAYER_COUNT] = {",
    ...catalog.layers.map((layer) =>
      `    {${layer.graphics}, ${layer.palette}, ${layer.bitsPerPixel}, ${layer.cycleType}, ${layer.cycle1Start}, ${layer.cycle1End}, ${layer.cycle2Start}, ${layer.cycle2End}, ${layer.cycleSpeed}, ${layer.effect}},`),
    "};",
    "",
    "const EbEffectSpec eb_effects[EB_EFFECT_COUNT] = {",
    ...catalog.effects.map((effect) =>
      `    {${effect.type}, ${effect.frequency}, ${effect.amplitude}, ${effect.compression}, ${effect.speed}},`),
    "};",
    "",
    "const uint16_t eb_palettes[EB_PALETTE_COUNT][16] = {",
    ...catalog.palettes.map((palette) => {
      const colors = [...palette.colors];
      while (colors.length < 16) colors.push(0);
      return `    {${colors.join(", ")}},`;
    }),
    "};",
    "",
    "uint8_t *eb_graphics_source(uint8_t index)",
    "{",
    "    switch (index) {",
    ...native.graphics.offsets.map((_, index) => {
      const name = String(index).padStart(3, "0");
      return `    case ${index}: return &gfx_${name};`;
    }),
    "    default: return &gfx_000;",
    "    }",
    "}",
    "",
    "uint8_t *eb_map_source(uint8_t index)",
    "{",
    "    switch (index) {",
    ...native.graphics.offsets.map((_, index) => {
      const name = String(index).padStart(3, "0");
      return `    case ${index}: return &map_${name};`;
    }),
    "    default: return &map_000;",
    "    }",
    "}",
    "",
  ];

  fs.writeFileSync(path.join(outputDirectory, "data.asm"), assembly.join("\n"));
  fs.writeFileSync(path.join(outputDirectory, "generated_assets.h"), declarations.join("\n"));
  fs.writeFileSync(path.join(outputDirectory, "generated_assets.c"), source.join("\n"));
  return { graphicsCount: lengths.length, layerCount: catalog.layers.length };
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const result = generate({
    layersPath: path.join(root, "data/layers.json"),
    nativePath: path.join(root, "data/native-data.json"),
    outputDirectory: path.join(root, "generated"),
  });
  console.log(`generated ${result.graphicsCount} graphics banks and ${result.layerCount} layers`);
}
