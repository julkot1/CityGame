#include "world/Chunk.h"
#include "world/EntityModelRegistry.h"

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

    // Helper to test if a neighbor tile is higher than the current height
    auto isHigher = [&](Int nx, Int nz, Float refH) -> bool {
        if(nx < 0 || nx >= Size || nz < 0 || nz >= Size) return false;
        const Tile& nt = tile(nx, nz);
        return nt.active && (nt.height > refH + 0.01f);
    };

    // Standard voxel ambient occlusion calculation for a vertex given two adjacent side neighbors and one diagonal corner neighbor
    auto vertexAO = [](bool side1, bool side2, bool corner) -> Float {
        if(side1 && side2) return 0.25f; // Deep corner between two intersecting walls
        int count = (side1 ? 1 : 0) + (side2 ? 1 : 0) + (corner ? 1 : 0);
        return 1.0f - Float(count) * 0.25f; // 1.0 (open), 0.75 (one neighbor), 0.50 (two neighbors)
    };

    for(Int z = 0; z < Size; ++z) {
        for(Int x = 0; x < Size; ++x) {
            const Tile& t = tile(x, z);
            if(!t.active) continue;

            const Float cx = Float(x) * tileSpacing;
            const Float cz = Float(z) * tileSpacing;
            const Float h = t.height;
            const Color3 baseCol = t.color;

            // Neighbor height tests for Top face AO
            bool nN  = isHigher(x, z - 1, h); // -Z (North)
            bool nS  = isHigher(x, z + 1, h); // +Z (South)
            bool nW  = isHigher(x - 1, z, h); // -X (West)
            bool nE  = isHigher(x + 1, z, h); // +X (East)
            bool nNW = isHigher(x - 1, z - 1, h);
            bool nNE = isHigher(x + 1, z - 1, h);
            bool nSW = isHigher(x - 1, z + 1, h);
            bool nSE = isHigher(x + 1, z + 1, h);

            // Compute AO for each corner of the top face:
            // v0: (-X, -Z) -> West & North
            // v1: (+X, -Z) -> East & North
            // v2: (+X, +Z) -> East & South
            // v3: (-X, +Z) -> West & South
            Float ao0 = vertexAO(nW, nN, nNW);
            Float ao1 = vertexAO(nE, nN, nNE);
            Float ao2 = vertexAO(nE, nS, nSE);
            Float ao3 = vertexAO(nW, nS, nSW);

            // Top (+Y) face
            UnsignedInt base = vertices.size();
            Vector3 topNormal{0.0f, 1.0f, 0.0f};
            vertices.push_back({{cx - halfSize, h, cz - halfSize}, topNormal, baseCol, ao0});
            vertices.push_back({{cx + halfSize, h, cz - halfSize}, topNormal, baseCol, ao1});
            vertices.push_back({{cx + halfSize, h, cz + halfSize}, topNormal, baseCol, ao2});
            vertices.push_back({{cx - halfSize, h, cz + halfSize}, topNormal, baseCol, ao3});

            // Flip quad triangulation if diagonal AO contrast would produce an interpolation crease
            if(ao0 + ao2 < ao1 + ao3) {
                indices.push_back(base + 1);
                indices.push_back(base + 0);
                indices.push_back(base + 3);
                indices.push_back(base + 1);
                indices.push_back(base + 3);
                indices.push_back(base + 2);
            } else {
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 1);
                indices.push_back(base + 0);
                indices.push_back(base + 3);
                indices.push_back(base + 2);
            }

            // Front (+Z): Only emit if neighbor in +Z direction is missing or lower
            Float frontH = (z < Size - 1) ? tile(x, z + 1).height : 0.0f;
            if(h > frontH) {
                base = vertices.size();
                Vector3 frontNormal{0.0f, 0.0f, 1.0f};
                // Wall vertices: base gets slight bottom contact AO (0.75), top vertices full or slight shade
                vertices.push_back({{cx - halfSize, frontH, cz + halfSize}, frontNormal, baseCol, 0.70f});
                vertices.push_back({{cx + halfSize, frontH, cz + halfSize}, frontNormal, baseCol, 0.70f});
                vertices.push_back({{cx + halfSize,      h, cz + halfSize}, frontNormal, baseCol, 1.0f});
                vertices.push_back({{cx - halfSize,      h, cz + halfSize}, frontNormal, baseCol, 1.0f});
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
                Vector3 backNormal{0.0f, 0.0f, -1.0f};
                vertices.push_back({{cx + halfSize, backH, cz - halfSize}, backNormal, baseCol, 0.70f});
                vertices.push_back({{cx - halfSize, backH, cz - halfSize}, backNormal, baseCol, 0.70f});
                vertices.push_back({{cx - halfSize,     h, cz - halfSize}, backNormal, baseCol, 1.0f});
                vertices.push_back({{cx + halfSize,     h, cz - halfSize}, backNormal, baseCol, 1.0f});
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
                Vector3 leftNormal{-1.0f, 0.0f, 0.0f};
                vertices.push_back({{cx - halfSize, leftH, cz - halfSize}, leftNormal, baseCol, 0.70f});
                vertices.push_back({{cx - halfSize, leftH, cz + halfSize}, leftNormal, baseCol, 0.70f});
                vertices.push_back({{cx - halfSize,     h, cz + halfSize}, leftNormal, baseCol, 1.0f});
                vertices.push_back({{cx - halfSize,     h, cz - halfSize}, leftNormal, baseCol, 1.0f});
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
                Vector3 rightNormal{1.0f, 0.0f, 0.0f};
                vertices.push_back({{cx + halfSize, rightH, cz + halfSize}, rightNormal, baseCol, 0.70f});
                vertices.push_back({{cx + halfSize, rightH, cz - halfSize}, rightNormal, baseCol, 0.70f});
                vertices.push_back({{cx + halfSize,      h, cz - halfSize}, rightNormal, baseCol, 1.0f});
                vertices.push_back({{cx + halfSize,      h, cz + halfSize}, rightNormal, baseCol, 1.0f});
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 0);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }
        }
    }

    // Build geometry for all chunk entities (e.g. Trees)
    for(const Entity& entity : _entities) {
        EntityModelRegistry::buildEntityMesh(entity, vertices, indices);
    }

    _vertexBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
    _vertexBuffer.setData(Containers::arrayView(vertices), GL::BufferUsage::StaticDraw);

    _indexBuffer = GL::Buffer{GL::Buffer::TargetHint::ElementArray};
    _indexBuffer.setData(Containers::arrayView(indices), GL::BufferUsage::StaticDraw);

    _mesh = GL::Mesh{};
    _mesh.setCount(indices.size())
         .addVertexBuffer(_vertexBuffer, 0,
            GL::Attribute<0, Vector3>{}, // Position
            GL::Attribute<1, Vector3>{}, // Normal
            GL::Attribute<2, Color3>{},  // Color
            GL::Attribute<3, Float>{})   // Ambient Occlusion
         .setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt);
    _meshGenerated = true;
}

}
