# Terrain LOD (Level of Detail) — Implementation Plan

## Problem

The terrain streamer currently renders all tiles at zoom 14 (each ~2446 m). This limits the visible range to roughly ±15 km from the player. Tiles far from the camera receive the same resolution as those right below the aircraft, wasting bandwidth, memory, and GPU fill-rate.

## Proposed Approach

Introduce three concentric LOD rings around the player, each using a different zoom level:

| Ring     | Zoom | Tile world-size | Grid extent      | Coverage radius   |
|----------|------|-----------------|------------------|-------------------|
| Close    | 14   | 2 446 m         | 7 × 7 (±3)      | ~8.6 km           |
| Medium   | 13   | 4 892 m         | 7 × 7 (±3)      | ~17.1 km          |
| Far      | 12   | 9 784 m         | 7 × 7 (±3)      | ~34.2 km          |

> Grid extents above are *proposed defaults*. They can be tuned later (ideally from config).

The rings are evaluated from highest zoom to lowest. A tile cell at a lower zoom is **skipped** if its world footprint is already fully covered by higher-zoom tiles, preventing overdraw.

### Coordinate relationship between zooms

Standard web-mercator doubling:

```
zoom 12 tile (x, z)  →  zoom 13 tiles (2x, 2z), (2x+1, 2z), (2x, 2z+1), (2x+1, 2z+1)
zoom 13 tile (x, z)  →  zoom 14 tiles (2x, 2z), …
```

Base offsets follow the same rule:

| Zoom | BASE_X | BASE_Z | TILE_SIZE (m) |
|------|--------|--------|---------------|
| 12   | 2 435  | 1 653  | 9 783.9       |
| 13   | 4 870  | 3 306  | 4 891.95      |
| 14   | 9 755  | 6 627  | 2 445.975     |

(BASE at zoom N = BASE at zoom 12 × 2^(N−12), matching the commented constants in the code.)

---

## Design Decisions (confirmed)

| Decision | Choice |
|----------|--------|
| Tile path format | Change to `assets/tiles/cache/{type}/{zoom}/{x}/{z}.png` — zoom becomes a format parameter |
| LOD transition | Keep old tile visible until new-LOD tile finishes async load, then swap |
| Mesh density | 256 × 256 subdivisions for all zoom levels |
| View distance | Larger than today — zoom-12 tiles extend coverage cheaply |

---

## Implementation Todos

### 1. Introduce `ZoomLevel` struct and LOD table

Create a small struct/array that holds per-zoom metadata:

```cpp
struct ZoomLevel {
    int zoom;           // 12, 13, 14
    Meter tile_size;    // world-space side length
    int base_x;
    int base_z;
    int radius;         // grid half-extent (e.g. 3 → 7×7)
};
```

Keep a `constexpr` or config-loaded array of these (ordered 14 → 13 → 12, highest first).

### 2. One `Model` per zoom level

Currently a single `Model` is created via `create_model()` using `TILE_SIZE`. With LOD, create one mesh per zoom (three total), each with 256 × 256 subdivisions but different physical sizes. Store them in a small `std::array<Model, 3>` or `std::map<int, Model>`.

The displacement shader and heightMap slot setup is identical across all three — only the plane size differs.

### 3. Update path format in `TilesDef` / `scenario.jsonc`

Change the path templates to accept zoom:

```jsonc
"tex_path": "assets/tiles/cache/texture/{}/{}/{}.png",   // zoom, x, z
"hmp_path": "assets/tiles/cache/heightmaps/{}/{}/{}.png"
```

Update `spawn_tile()` to format with three args: `std::vformat(tiles.tex_path, std::make_format_args(zoom, tx, tz))`.

### 4. Extend tile identity to include zoom

Currently tiles are tracked by `TileCoord = std::pair<int, int>`. Extend to:

```cpp
struct TileKey {
    int zoom;
    int x;
    int z;
};
```

This becomes the key for `active_tiles` map and for resource IDs (`get_tile_id`, `get_tex_id`, `get_height_id` must include zoom).

### 5. Add zoom to `TerrainChunk` / `AsyncTileLoad` components

`AsyncTileLoad` and `TerrainChunk` need to store their zoom level so the render pass picks the correct `Model` (mesh size).

### 6. Rewrite `update()` — multi-zoom tile selection

The core algorithm becomes:

```
for each zoom in [14, 13, 12]:
    compute player's tile coord at this zoom
    collect required tile coords for this zoom's ring
    remove coords whose footprint is already covered by a higher zoom
    add remaining to required set

evict active tiles not in the total required set (with transition grace — see §7)
spawn new tiles that are required but not yet active
```

**Coverage check:** a zoom-13 tile at (x, z) is "covered" if all four child zoom-14 tiles `(2x, 2z)...(2x+1, 2z+1)` are in the zoom-14 required set. Same logic for zoom-12 vs zoom-13.

### 7. Graceful LOD transitions

When a tile's required zoom changes (e.g., a zoom-13 area now needs zoom-14):

1. Spawn the new higher-zoom tiles (async load begins).
2. Keep the old lower-zoom tile **alive and rendering** (do not destroy yet).
3. Once **all** higher-zoom tiles that overlap the old tile have finished loading (have `TerrainChunk` component, not `AsyncTileLoad`), destroy the old tile.

This requires a small bookkeeping structure — e.g., a `PendingReplacement` component on the old tile that tracks which new entities must be ready before it can be destroyed.

### 8. Update `stream()` (render)

The render loop already iterates `TerrainChunk + Position3D`. Changes:

- Read the zoom from `TerrainChunk` and select the matching `Model` from the per-zoom model array.
- Everything else (texture binding, draw) stays the same.

### 9. Update `process_loaded_chunks()`

Minor: include zoom in the `TerrainChunk` emplacement. Check `PendingReplacement` targets and destroy superseded tiles when all replacements are loaded.

### 10. Configuration / tunability

Expose the LOD ring radii (and optionally the zoom set) in `scenario.jsonc` or `config.jsonc` so it can be tuned without recompilation:

```jsonc
"lod": [
    { "zoom": 14, "radius": 3 },
    { "zoom": 13, "radius": 3 },
    { "zoom": 12, "radius": 3 }
]
```

---

## Things that stay the same

- The displacement shader, heightmap binding, and sky/fog shaders are unchanged.
- `TerrainHeight` and ground-height sampling still work — only the closest (zoom 14) tiles participate.
- Async image loading pattern (`std::async` → `process_loaded_chunks`) is preserved.
- The `TilesDef` struct keeps all its existing fields; we only add to it.

## Edge cases / things to watch

| Concern | Mitigation |
|---------|------------|
| Z-fighting between overlapping LOD tiles during transition | Old tile is slightly below (its `lowest` Y) — should be occluded. If visible, add a small Y offset to lower-zoom tiles. |
| Missing tile files at a zoom level | `LoadImage` already returns empty on missing file; treat as transparent/skip. |
| Memory spike during transition (both old and new tiles alive) | Bounded — at most one ring's worth of tiles duplicated briefly. |
| Heightmap sampling for ground check | Always sample from zoom-14 tiles only (highest detail). If no zoom-14 tile is loaded yet, fall back to lower zoom. |

## Rough tile count estimate

| Ring   | Grid  | Tiles | Note |
|--------|-------|-------|------|
| z14    | 7×7   | 49    | Same mesh cost, high-res texture |
| z13    | 7×7   | 49    | Minus ~12 covered by z14 → ~37 |
| z12    | 7×7   | 49    | Minus ~12 covered by z13 → ~37 |
| **Total** | | **~123** | Down from 169 today, but covers ~4× the area |

---

## Suggested implementation order

1. `ZoomLevel` struct + LOD table + per-zoom models (todo 1, 2)
2. `TileKey` + update identifiers (todo 4, 5)
3. Path format change (todo 3)
4. Multi-zoom `update()` (todo 6) — core LOD logic
5. Graceful transitions (todo 7)
6. Render updates (todo 8, 9)
7. Config/tunability (todo 10)
8. Testing & tuning ring sizes
