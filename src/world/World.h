#pragma once

#include <map>
#include <memory>
#include <utility>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/Chunk.h"
#include "world/WorldGenerator.h"

namespace Magnum {

class World {
    public:
        static constexpr Int WorldChunkWidth = 8;
        static constexpr Int WorldChunkHeight = 8;

        explicit World(Float tileSpacing = 1.05f, std::shared_ptr<WorldGenerator> generator = nullptr);

        void setGenerator(std::shared_ptr<WorldGenerator> generator);
        WorldGenerator* generator() { return _generator.get(); }
        const WorldGenerator* generator() const { return _generator.get(); }

        void init();

        Chunk& getOrCreateChunk(const Vector2i& chunkCoord);
        Chunk* chunk(const Vector2i& chunkCoord);
        const Chunk* chunk(const Vector2i& chunkCoord) const;

        Tile* tileAt(Int worldX, Int worldZ);
        const Tile* tileAt(Int worldX, Int worldZ) const;

        const std::map<std::pair<Int, Int>, Chunk>& chunks() const { return _chunks; }

        Float tileSpacing() const { return _tileSpacing; }
        Float chunkSizeInWorldUnits() const { return Float(Chunk::Size) * _tileSpacing; }

        Float minWorldX() const { return -Float(WorldChunkWidth) * 0.5f * chunkSizeInWorldUnits(); }
        Float maxWorldX() const { return  Float(WorldChunkWidth) * 0.5f * chunkSizeInWorldUnits(); }
        Float minWorldZ() const { return -Float(WorldChunkHeight) * 0.5f * chunkSizeInWorldUnits(); }
        Float maxWorldZ() const { return  Float(WorldChunkHeight) * 0.5f * chunkSizeInWorldUnits(); }

        static Vector2i worldToChunkCoord(Int worldX, Int worldZ);
        static Vector2i worldToLocalTileCoord(Int worldX, Int worldZ);

        // Raycasting to find clicked tile
        struct RaycastHit {
            bool hit{false};
            Vector2i chunkCoord{0, 0};
            Vector2i localTileCoord{0, 0};
            Int worldTileX{0};
            Int worldTileZ{0};
            Vector3 worldPoint{0.0f};
        };

        RaycastHit raycastTile(const Vector3& rayOrigin, const Vector3& rayDir) const;

    private:
        Float _tileSpacing{1.05f};
        std::shared_ptr<WorldGenerator> _generator;
        std::map<std::pair<Int, Int>, Chunk> _chunks;
};

}
