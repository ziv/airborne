#!/usr/bin/env node
import * as process from "node:process";
import {createWriteStream, mkdirSync, existsSync} from 'fs';
import {dirname} from 'path';
import {Readable} from 'stream';
import {finished} from 'stream/promises';

const argv = process.argv.slice(2);
if (argv.length !== 3) {
    console.error("Usage: download_map.mjs <zoom> <x> <z>");
    process.exit(1);
}

const [zoom, x, z] = argv;

const token = process.env["TOMTOM_TOKEN"];
if (!token) {
    console.error("TOMTOM_TOKEN environment variable is not set");
    process.ex(2);
}

async function download_map(zoom, x, z, token) {
    const url = `https://api.tomtom.com/map/1/tile/basic/main/${zoom}/${x}/${z}.png?tileSize=256&view=Unified&language=NGT&key=${token}`;
    const path = `assets/tiles/cache/map/${zoom}/${x}/${z}.png`;

    if (existsSync(path)) {
        console.error(`map already exists at ${path}, skipping`);
        return;
    }

    const response = await fetch(url);
    if (!response.ok) {
        console.error(`failed to download map for tile ${zoom}/${x}/${z}: ${response.status} ${response.statusText}`);
        return;
    }

    mkdirSync(dirname(path), {recursive: true});
    const fileStream = createWriteStream(path);
    await finished(Readable.fromWeb(response.body).pipe(fileStream));
    console.error(`downloaded map to path ${path}`)
}


await download_map(zoom, x, z, token);