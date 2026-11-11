#pragma once

#include <array>
#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/Tile.h"
#include "world/Entity.h"

namespace Magnum {

struct ChunkVertex {
    Vector3 position;
    Vector3 normal;
    Color3 color;
    Float ao; // Ambient occlusion factor (0.0 = deep shadow corner, 1.0 = fully exposed)
};

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

        // Entity Management
        const std::vector<Entity>& entities() const { return _entities; }
        std::vector<Entity>& entities() { return _entities; }
        void addEntity(const Entity& entity) { _entities.push_back(entity); }
        void clearEntities() { _entities.clear(); }
        bool removeEntityAt(Int x, Int z) {
            for(auto it = _entities.begin(); it != _entities.end(); ++it) {
                if(Math::abs(it->position.x() - Float(x)) < 0.45f &&
                   Math::abs(it->position.z() - Float(z)) < 0.45f) {
                    _entities.erase(it);
                    return true;
                }
            }
            return false;
        }
        Entity* entityAt(Int x, Int z) {
            for(auto& e : _entities) {
                if(Math::abs(e.position.x() - Float(x)) < 0.45f &&
                   Math::abs(e.position.z() - Float(z)) < 0.45f) {
                    return &e;
                }
            }
            return nullptr;
        }
        const Entity* entityAt(Int x, Int z) const {
            for(const auto& e : _entities) {
                if(Math::abs(e.position.x() - Float(x)) < 0.45f &&
                   Math::abs(e.position.z() - Float(z)) < 0.45f) {
                    return &e;
                }
            }
            return nullptr;
        }

        void buildMesh(Float tileSpacing = 0.0f);
        bool hasMesh() const { return _meshGenerated; }
        GL::Mesh& mesh() { return _mesh; }
        GL::Mesh& mesh() const { return _mesh; }

    private:
        Vector2i _chunkCoord{0, 0};
        std::array<Tile, Size * Size> _tiles;
        std::vector<Entity> _entities;

        mutable GL::Mesh _mesh;
        GL::Buffer _vertexBuffer{NoCreate};
        GL::Buffer _indexBuffer{NoCreate};
        bool _meshGenerated{false};
};

}
