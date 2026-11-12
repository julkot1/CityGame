#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>

namespace Magnum {

/**
 * GPU handles for a single chunk's rendered mesh.
 * Owned and manipulated exclusively by the Render thread.
 */
struct RenderChunk {
    Vector2i chunkCoord{0, 0};
    GL::Mesh mesh{NoCreate};
    GL::Buffer vertexBuffer{NoCreate};
    GL::Buffer indexBuffer{NoCreate};
    bool hasMesh{false};
    UnsignedInt indexCount{0};
};

}
