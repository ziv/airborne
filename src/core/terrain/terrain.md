# Terrain Streamer

- Game coordinates (x,y,z) converted into terrain coordinates.
- We have point x=0,z=0 that lead to our terrain chunk (0+offset,0+offset) in minimal zoom level (zoom 10)
- The offset is arbitrary.
- Each texture/heightmap chunk size is 256x256.

```c++
const player_absolute_postion = player.position - offset;
const tile_size = get_tile_size_meters(10); // zoom 10


// find the chunk coordinates of the player
int chunk_x = static_cast<int>(std::floor(player_absolute_postion.x / tile_size)) + BASE_X;
int chunk_z = static_cast<int>(std::floor(player_absolute_postion.z / tile_size)) + BASE_Z;


// find the cunks around the player
for (int x = chunk_x - 1; x <= chunk_x + 1; ++x) {
    for (int z = chunk_z - 1; z <= chunk_z + 1; ++z) {
        // load the chunk at (x, z)
    }
}

```

```c++
constexpr double EARTH_CIRCUMFERENCE = 40075016.686;
inline double get_tile_size_meters(int zoom) {
    double tilesPerSide = static_cast<double>(1ULL << zoom);
    return EARTH_CIRCUMFERENCE / tilesPerSide;
}

double zoom10Width = get_tile_size_meters(10);

constxpr int CHUNK_SIZE = 256;
constxpr int BASE_X = 9775;
constxpr int BASE_Z = 6648;
constxpr float TILE_REAL_WORLD_SIZE = 2445.985f; // zoom 14 
constxpr float TILE_REAL_WORLD_SIZE = 39135.0f; // zoom 10 
```