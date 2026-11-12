#pragma once

#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

struct ChunkVertex {
    Vector3 position;
    Vector3 normal;
    Color3 color;
    Float ao; // Ambient occlusion / hardware shader tag
};

struct ChunkMeshData {
    Vector2i chunkCoord{0, 0};
    std::vector<ChunkVertex> vertices;
    std::vector<UnsignedInt> indices;
};

}
