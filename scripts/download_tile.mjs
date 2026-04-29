#!/usr/bin/env node
import * as process from "node:process";
import {createWriteStream, mkdirSync, existsSync} from 'fs';
import {dirname} from 'path';
import {Readable} from 'stream';
import {finished} from 'stream/promises';

const argv = process.argv.slice(2);
if (argv.length !== 4) {
    console.error("Usage: node download_tile.mjs <zoom> <x> <z>");
    process.exit(1);
}

const [zoom, x, z, p] = argv;

const mapbox_token = process.env["MAPBOX_TOKEN"];
if (!mapbox_token) {
    console.error("MAPBOX_TOKEN environment variable is not set");
    process.ex(2);
}

let url;

if (p.includes("texture")) {
    url = `https://api.mapbox.com/v4/mapbox.satellite/${zoom}/${x}/${z}.png?access_token=${mapbox_token}`;
} else if (p.includes("heightmap")) {
    url = `https://api.mapbox.com/v4/mapbox.terrain-rgb/${zoom}/${x}/${z}.pngraw?access_token=${mapbox_token}`;
} else if (p.includes("map")) {
    url = `todo`;
}

if (existsSync(p)) {
    console.error(`texture already exists at ${p}, skipping`);
    process.exit(0);
}

const response = await fetch(url);
if (!response.ok) {
    console.error(`failed to download texture for tile ${zoom}/${x}/${z}: ${response.status} ${response.statusText}`);
    process.exit(3);
}

mkdirSync(dirname(p), {recursive: true});
const fileStream = createWriteStream(p);
await finished(Readable.fromWeb(response.body).pipe(fileStream));
console.error(`tile ${p} downloaded`);


//
// async function download_texture(zoom, x, z, token) {
//     const url = `https://api.mapbox.com/v4/mapbox.satellite/${zoom}/${x}/${z}.png?access_token=${token}`;
//     const path = `assets/tiles/cache/texture/${zoom}/${x}/${z}.png`;
//
//     if (existsSync(path)) {
//         console.error(`texture already exists at ${path}, skipping`);
//         return;
//     }
//
//     const response = await fetch(url);
//     if (!response.ok) {
//         console.error(`failed to download texture for tile ${zoom}/${x}/${z}: ${response.status} ${response.statusText}`);
//         return;
//     }
//
//     mkdirSync(dirname(path), {recursive: true});
//     const fileStream = createWriteStream(path);
//     await finished(Readable.fromWeb(response.body).pipe(fileStream));
//     console.error(`downloaded texture to path ${path}`)
// }
//
// async function download_heightmap(zoom, x, z, token) {
//     const url = `https://api.mapbox.com/v4/mapbox.terrain-rgb/${zoom}/${x}/${z}.pngraw?access_token=${token}`;
//     const path = `assets/tiles/cache/heightmaps/${zoom}/${x}/${z}.png`;
//
//     if (existsSync(path)) {
//         console.error(`heightmap already exists at ${path}, skipping`);
//         return;
//     }
//
//     const response = await fetch(url);
//     if (!response.ok) {
//         console.error(`failed to download heightmap for tile ${zoom}/${x}/${z}: ${response.status} ${response.statusText}`);
//         return;
//     }
//
//     mkdirSync(dirname(path), {recursive: true});
//     const fileStream = createWriteStream(path);
//     await finished(Readable.fromWeb(response.body).pipe(fileStream));
//     console.error(`downloaded heightmap to path ${path}`)
// }
//
// await Promise.all([
//     download_texture(zoom, x, z, token),
//     download_heightmap(zoom, x, z, token)
// ]);