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

const tomtom_token = process.env["TOMTOM_TOKEN"];
if (!tomtom_token) {
    console.error("TOMTOM_TOKEN environment variable is not set");
    process.ex(2);
}

let url;

// textures
if (p.includes("texture")) {
    url = `https://api.mapbox.com/v4/mapbox.satellite/${zoom}/${x}/${z}.png?access_token=${mapbox_token}`;
}
// heightmap
else if (p.includes("heightmap")) {
    url = `https://api.mapbox.com/v4/mapbox.terrain-rgb/${zoom}/${x}/${z}.pngraw?access_token=${mapbox_token}`;
}
// roads
else if (p.includes("map")) {
    const theme = "main";
    url = `https://api.tomtom.com/map/1/tile/basic/${theme}/${zoom}/${x}/${z}.png?tileSize=256&view=Unified&language=NGT&key=${tomtom_token}`;
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
