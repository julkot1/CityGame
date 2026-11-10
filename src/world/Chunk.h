#pragma once

#include <array>
#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/Tile.h"

namespace Magnum {

struct ChunkVertex {
    Vector3 position;
    Color3 color;
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

        void buildMesh(Float tileSpacing = 0.0f);
        bool hasMesh() const { return _meshGenerated; }
        GL::Mesh& mesh() { return _mesh; }
        GL::Mesh& mesh() const { return _mesh; }

    private:
        Vector2i _chunkCoord{0, 0};
        std::array<Tile, Size * Size> _tiles;

        mutable GL::Mesh _mesh;
        GL::Buffer _vertexBuffer{NoCreate};
        GL::Buffer _indexBuffer{NoCreate};
        bool _meshGenerated{false};
};

}
