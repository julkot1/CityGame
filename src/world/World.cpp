#include "world/World.h"
#include <cmath>

namespace Magnum {

World::World(Float tileSpacing):
    _tileSpacing{tileSpacing}
{
    init();
}

void World::init() {
    _chunks.clear();

    const Int minCX = -WorldChunkWidth / 2;
    const Int maxCX = minCX + WorldChunkWidth - 1;
    const Int minCZ = -WorldChunkHeight / 2;
    const Int maxCZ = minCZ + WorldChunkHeight - 1;

    for(Int cz = minCZ; cz <= maxCZ; ++cz) {
        for(Int cx = minCX; cx <= maxCX; ++cx) {
            Chunk& c = getOrCreateChunk({cx, cz});
            c.buildMesh(_tileSpacing);
        }
    }
}

Vector2i World::worldToChunkCoord(Int worldX, Int worldZ) {
    Int cx = worldX >= 0 ? (worldX / Chunk::Size) : ((worldX - Chunk::Size + 1) / Chunk::Size);
    Int cz = worldZ >= 0 ? (worldZ / Chunk::Size) : ((worldZ - Chunk::Size + 1) / Chunk::Size);
    return {cx, cz};
}

Vector2i World::worldToLocalTileCoord(Int worldX, Int worldZ) {
    Int lx = (worldX % Chunk::Size + Chunk::Size) % Chunk::Size;
    Int lz = (worldZ % Chunk::Size + Chunk::Size) % Chunk::Size;
    return {lx, lz};
}

Chunk& World::getOrCreateChunk(const Vector2i& chunkCoord) {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    if(it != _chunks.end()) {
        return it->second;
    }
    auto inserted = _chunks.emplace(key, Chunk{chunkCoord});
    return inserted.first->second;
}

Chunk* World::chunk(const Vector2i& chunkCoord) {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    return it != _chunks.end() ? &it->second : nullptr;
}

const Chunk* World::chunk(const Vector2i& chunkCoord) const {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    return it != _chunks.end() ? &it->second : nullptr;
}

Tile* World::tileAt(Int worldX, Int worldZ) {
    Vector2i cc = worldToChunkCoord(worldX, worldZ);
    Chunk* ch = chunk(cc);
    if(!ch) return nullptr;
    Vector2i local = worldToLocalTileCoord(worldX, worldZ);
    return &ch->tile(local.x(), local.y());
}

const Tile* World::tileAt(Int worldX, Int worldZ) const {
    Vector2i cc = worldToChunkCoord(worldX, worldZ);
    const Chunk* ch = chunk(cc);
    if(!ch) return nullptr;
    Vector2i local = worldToLocalTileCoord(worldX, worldZ);
    return &ch->tile(local.x(), local.y());
}

}
