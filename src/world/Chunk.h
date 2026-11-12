#pragma once

#include <array>
#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/Tile.h"
#include "world/Entity.h"
#include "world/ChunkMeshData.h"
#include "building/BuildingCatalog.h"

namespace Magnum {

class Chunk {
    public:
        static constexpr Int Size = 32;

        explicit Chunk(const Vector2i& chunkCoord = {0, 0});
        Chunk(Chunk&&) noexcept = default;
        Chunk& operator=(Chunk&&) noexcept = default;

        Vector2i chunkCoord() const { return _chunkCoord; }
        Vector3 worldPosition(Float tileSpacing = 0.0f) const;

        Tile& tile(Int x, Int z);
        const Tile& tile(Int x, Int z) const;
        const std::array<Tile, Size * Size>& tiles() const { return _tiles; }
        std::array<Tile, Size * Size>& tiles() { return _tiles; }

        // Entity Management (uses BuildingCatalog to support arbitrary NxM footprints)
        const std::vector<Entity>& entities() const { return _entities; }
        std::vector<Entity>& entities() { return _entities; }
        void addEntity(const Entity& entity) { _entities.push_back(entity); }
        void clearEntities() { _entities.clear(); }
        bool removeEntityAt(Int x, Int z) {
            for(auto it = _entities.begin(); it != _entities.end(); ++it) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(it->type, it->data & 3u);
                if(Math::abs(it->position.x() - Float(x)) <= halfExt.x() &&
                   Math::abs(it->position.z() - Float(z)) <= halfExt.y()) {
                    _entities.erase(it);
                    return true;
                }
            }
            return false;
        }
        Entity* entityAt(Int x, Int z) {
            for(auto& e : _entities) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(e.type, e.data & 3u);
                if(Math::abs(e.position.x() - Float(x)) <= halfExt.x() &&
                   Math::abs(e.position.z() - Float(z)) <= halfExt.y()) {
                    return &e;
                }
            }
            return nullptr;
        }
        const Entity* entityAt(Int x, Int z) const {
            for(const auto& e : _entities) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(e.type, e.data & 3u);
                if(Math::abs(e.position.x() - Float(x)) <= halfExt.x() &&
                   Math::abs(e.position.z() - Float(z)) <= halfExt.y()) {
                    return &e;
                }
            }
            return nullptr;
        }

        // Generates pure CPU geometry (vertices, normals, colors, AO tags, indices) without touching OpenGL
        ChunkMeshData buildCpuMesh(Float tileSpacing = 0.0f) const;

    private:
        Vector2i _chunkCoord{0, 0};
        std::array<Tile, Size * Size> _tiles;
        std::vector<Entity> _entities;
};

}
