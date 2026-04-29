#!/usr/bin/env node
import * as process from "node:process";
import {createWriteStream, mkdirSync, existsSync} from 'fs';
import {dirname} from 'path';
import {Readable} from 'stream';
import {finished} from 'stream/promises';

const zoom = 14;
const minx = 9612;
const maxx = 9823;
const miny = 6627;
const maxy = 6667;

