#!/usr/bin/env node
import * as process from "node:process";
import {createWriteStream, mkdirSync, existsSync} from "node:fs";
import {dirname} from "node:path";
import {Readable} from "node:stream";
import {finished} from "node:stream/promises";

const ZOOM = 10;
const minx = 609;
const maxx = 614;
const miny = 411;
const maxy = 412;

for (let zoom = 10; zoom <= 14; ++zoom) {
    for (let x = minx; x <= maxx; ++x) {
        for (let z = miny; z <= maxy; ++z) {
            const texture = `assets/texture/${zoom}/${x}/${z}.png`;
            const heightmap = `assets/heightmap/${zoom}/${x}/${z}.png`;
            const map = `assets/map/${zoom}/${x}/${z}.png`;

            if (!existsSync(texture)) {
                // download it
            }

            if (!existsSync(heightmap)) {
                // download it
            }

            if (!existsSync(map)) {
                // download it
            }
        }
    }
}

