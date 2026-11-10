#include "world/Chunk.h"

#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/ArrayViewStl.h>
#include <Corrade/Utility/DebugAssert.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <random>
using namespace std;
namespace Magnum {

using namespace Math::Literals;

Chunk::Chunk(const Vector2i& chunkCoord):
    _chunkCoord{chunkCoord}
{
    for(Int z = 0; z < Size; ++z) {
        for(Int x = 0; x < Size; ++x) {
            Tile& t = tile(x, z);
            t.type = TileType::Grass;
            t.color = 0x689d6a_rgbf;
            t.height = 1.0f;
            t.active = true;
        }
    }
}

Tile& Chunk::tile(Int x, Int z) {
    CORRADE_INTERNAL_ASSERT(x >= 0 && x < Size && z >= 0 && z < Size);
    return _tiles[z * Size + x];
}

const Tile& Chunk::tile(Int x, Int z) const {
    CORRADE_INTERNAL_ASSERT(x >= 0 && x < Size && z >= 0 && z < Size);
    return _tiles[z * Size + x];
}

Vector3 Chunk::worldPosition(Float tileSpacing) const {
    return Vector3{
        Float(_chunkCoord.x() * Size) * tileSpacing,
        0.0f,
        Float(_chunkCoord.y() * Size) * tileSpacing
    };
}

void Chunk::buildMesh(Float tileSpacing) {
    std::vector<ChunkVertex> vertices;
    std::vector<UnsignedInt> indices;
    vertices.reserve(Size * Size * 24);
    indices.reserve(Size * Size * 36);

    const Float halfSize = 0.5f;

    for(Int z = 0; z < Size; ++z) {
        for(Int x = 0; x < Size; ++x) {
            const Tile& t = tile(x, z);
            if(!t.active) continue;

            const Float cx = Float(x) * tileSpacing;
            const Float cz = Float(z) * tileSpacing;
            const Float h = t.height;
            const Color3 baseCol = t.color;

            const Color3 topCol   = baseCol * 1.00f;
            const Color3 frontCol = baseCol * 0.85f;
            const Color3 backCol  = baseCol * 0.70f;
            const Color3 leftCol  = baseCol * 0.75f;
            const Color3 rightCol = baseCol * 0.80f;

            UnsignedInt base = vertices.size();
            vertices.push_back({{cx - halfSize, h, cz - halfSize}, topCol});
            vertices.push_back({{cx + halfSize, h, cz - halfSize}, topCol});
            vertices.push_back({{cx + halfSize, h, cz + halfSize}, topCol});
            vertices.push_back({{cx - halfSize, h, cz + halfSize}, topCol});
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            indices.push_back(base + 0);
            indices.push_back(base + 3);
            indices.push_back(base + 2);

            // Front (+Z): Only emit if neighbor in +Z direction is missing or lower
            Float frontH = (z < Size - 1) ? tile(x, z + 1).height : 0.0f;
            if(h > frontH) {
                base = vertices.size();
                vertices.push_back({{cx - halfSize, frontH, cz + halfSize}, frontCol});
                vertices.push_back({{cx + halfSize, frontH, cz + halfSize}, frontCol});
                vertices.push_back({{cx + halfSize,      h, cz + halfSize}, frontCol});
                vertices.push_back({{cx - halfSize,      h, cz + halfSize}, frontCol});
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }

            // Back (-Z): Only emit if neighbor in -Z direction is missing or lower
            Float backH = (z > 0) ? tile(x, z - 1).height : 0.0f;
            if(h > backH) {
                base = vertices.size();
                vertices.push_back({{cx + halfSize, backH, cz - halfSize}, backCol});
                vertices.push_back({{cx - halfSize, backH, cz - halfSize}, backCol});
                vertices.push_back({{cx - halfSize,     h, cz - halfSize}, backCol});
                vertices.push_back({{cx + halfSize,     h, cz - halfSize}, backCol});
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }

            // Left (-X): Only emit if neighbor in -X direction is missing or lower
            Float leftH = (x > 0) ? tile(x - 1, z).height : 0.0f;
            if(h > leftH) {
                base = vertices.size();
                vertices.push_back({{cx - halfSize, leftH, cz - halfSize}, leftCol});
                vertices.push_back({{cx - halfSize, leftH, cz + halfSize}, leftCol});
                vertices.push_back({{cx - halfSize,     h, cz + halfSize}, leftCol});
                vertices.push_back({{cx - halfSize,     h, cz - halfSize}, leftCol});
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }

            // Right (+X): Only emit if neighbor in +X direction is missing or lower
            Float rightH = (x < Size - 1) ? tile(x + 1, z).height : 0.0f;
            if(h > rightH) {
                base = vertices.size();
                vertices.push_back({{cx + halfSize, rightH, cz + halfSize}, rightCol});
                vertices.push_back({{cx + halfSize, rightH, cz - halfSize}, rightCol});
                vertices.push_back({{cx + halfSize,      h, cz - halfSize}, rightCol});
                vertices.push_back({{cx + halfSize,      h, cz + halfSize}, rightCol});
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }
        }
    }

    _vertexBuffer = GL::Buffer{Containers::arrayView(vertices)};
    _indexBuffer = GL::Buffer{Containers::arrayView(indices)};
    _mesh = GL::Mesh{};
    _mesh.setCount(indices.size())
         .addVertexBuffer(_vertexBuffer, 0,
            Shaders::VertexColorGL3D::Position{},
            Shaders::VertexColorGL3D::Color3{})
         .setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt);
    _meshGenerated = true;
}

}
