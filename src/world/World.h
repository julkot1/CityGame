#pragma once

#include <map>
#include <utility>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/Chunk.h"

namespace Magnum {

class World {
    public:
        // World size in chunks along one dimension (e.g. 8x8 chunks = 256x256 tiles)
        static constexpr Int WorldChunkWidth = 8;
        static constexpr Int WorldChunkHeight = 8;

        explicit World(Float tileSpacing = 1.05f);

        void init();

        Chunk& getOrCreateChunk(const Vector2i& chunkCoord);
        Chunk* chunk(const Vector2i& chunkCoord);
        const Chunk* chunk(const Vector2i& chunkCoord) const;

        Tile* tileAt(Int worldX, Int worldZ);
        const Tile* tileAt(Int worldX, Int worldZ) const;

        const std::map<std::pair<Int, Int>, Chunk>& chunks() const { return _chunks; }

        Float tileSpacing() const { return _tileSpacing; }
        Float chunkSizeInWorldUnits() const { return Float(Chunk::Size) * _tileSpacing; }

        // Total world bounds in world units (minX, minZ, maxX, maxZ)
        Float minWorldX() const { return -Float(WorldChunkWidth) * 0.5f * chunkSizeInWorldUnits(); }
        Float maxWorldX() const { return  Float(WorldChunkWidth) * 0.5f * chunkSizeInWorldUnits(); }
        Float minWorldZ() const { return -Float(WorldChunkHeight) * 0.5f * chunkSizeInWorldUnits(); }
        Float maxWorldZ() const { return  Float(WorldChunkHeight) * 0.5f * chunkSizeInWorldUnits(); }

        static Vector2i worldToChunkCoord(Int worldX, Int worldZ);
        static Vector2i worldToLocalTileCoord(Int worldX, Int worldZ);

    private:
        Float _tileSpacing{1.05f};
        std::map<std::pair<Int, Int>, Chunk> _chunks;
};

}
