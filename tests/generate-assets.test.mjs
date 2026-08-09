import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";

import { expandTo4Bpp, generate, normalizeMap } from "../tools/generate-assets.mjs";

const twoBitTile = Buffer.from(Array.from({ length: 16 }, (_, index) => index + 1));
const expanded = expandTo4Bpp(twoBitTile, 2);
assert.equal(expanded.length, 32);
assert.deepEqual(expanded.subarray(0, 16), twoBitTile);
assert.deepEqual(expanded.subarray(16), Buffer.alloc(16));

const map = Buffer.alloc(2048);
map.writeUInt16LE(0xffff, 0);
const normalized = normalizeMap(map);
assert.equal(normalized.readUInt16LE(0), 0xe3ff);

const temporary = fs.mkdtempSync(path.join(os.tmpdir(), "earthbound-snes-assets-"));
const result = generate({
  layersPath: new URL("../data/layers.json", import.meta.url),
  nativePath: new URL("../data/native-data.json", import.meta.url),
  outputDirectory: temporary,
});
assert.deepEqual(result, { graphicsCount: 103, layerCount: 327 });
assert.equal(fs.statSync(path.join(temporary, "assets/gfx_000.bin")).size, 32);
assert.equal(fs.statSync(path.join(temporary, "assets/map_000.bin")).size, 2048);
assert.match(fs.readFileSync(path.join(temporary, "data.asm"), "utf8"), /gfx_102:/);
assert.match(fs.readFileSync(path.join(temporary, "generated_assets.c"), "utf8"), /case 102: return &gfx_102;/);

fs.rmSync(temporary, { recursive: true });
console.log("asset generator tests passed");
