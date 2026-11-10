#include "rendering/Renderer.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

using namespace Math::Literals;

Renderer::Renderer() {
    GL::Renderer::setClearColor(0xa5c9ea_rgbf);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
}

void Renderer::setViewport(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
}

void Renderer::render(const World& world, const Camera& camera) {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    const Matrix4 viewProjection = camera.viewProjectionMatrix();
    const Float chunkWorldSize = world.chunkSizeInWorldUnits();

    // 1 draw call per chunk!
    for(const auto& entry : world.chunks()) {
        const Chunk& chunk = entry.second;
        if(!chunk.hasMesh()) continue;

        Vector3 chunkPos{
            Float(chunk.chunkCoord().x()) * chunkWorldSize,
            0.0f,
            Float(chunk.chunkCoord().y()) * chunkWorldSize
        };

        _shader.setTransformationProjectionMatrix(viewProjection * Matrix4::translation(chunkPos));
        _shader.draw(chunk.mesh());
    }
}

}
