#include "rendering/Renderer.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/ArrayViewStl.h>
#include <vector>

namespace Magnum {

using namespace Math::Literals;

Renderer::Renderer() {
    GL::Renderer::setClearColor(0xa5c9ea_rgbf);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(
        GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha
    );

    // Build unit cube mesh for hover highlight
    const Float half = 0.5f;
    std::vector<Vector3> vertices = {
        // Top
        {-half, 1.0f, -half}, {half, 1.0f, -half}, {half, 1.0f, half}, {-half, 1.0f, half},
        // Front
        {-half, 0.0f,  half}, {half, 0.0f,  half}, {half, 1.0f, half}, {-half, 1.0f, half},
        // Back
        { half, 0.0f, -half}, {-half, 0.0f, -half}, {-half, 1.0f, -half}, {half, 1.0f, -half},
        // Left
        {-half, 0.0f, -half}, {-half, 0.0f,  half}, {-half, 1.0f, half}, {-half, 1.0f, -half},
        // Right
        { half, 0.0f,  half}, { half, 0.0f, -half}, { half, 1.0f, -half}, { half, 1.0f, half}
    };

    std::vector<UnsignedInt> indices;
    for(UnsignedInt i = 0; i < 5; ++i) {
        UnsignedInt b = i * 4;
        indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 1);
        indices.push_back(b + 0); indices.push_back(b + 3); indices.push_back(b + 2);
    }

    _highlightVertexBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
    _highlightVertexBuffer.setData(Containers::arrayView(vertices), GL::BufferUsage::StaticDraw);

    _highlightIndexBuffer = GL::Buffer{GL::Buffer::TargetHint::ElementArray};
    _highlightIndexBuffer.setData(Containers::arrayView(indices), GL::BufferUsage::StaticDraw);

    _highlightMesh.setPrimitive(GL::MeshPrimitive::Triangles)
                  .setCount(indices.size())
                  .addVertexBuffer(_highlightVertexBuffer, 0, Shaders::FlatGL3D::Position{})
                  .setIndexBuffer(GL::Buffer::wrap(_highlightIndexBuffer.id(), GL::Buffer::TargetHint::ElementArray), 0, GL::MeshIndexType::UnsignedInt);
}

void Renderer::setViewport(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
}

void Renderer::render(const World& world, const Camera& camera, const World::RaycastHit& hoverHit) {
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

    // Render hover highlight overlay if hovering a valid tile
    if(hoverHit.hit) {
        const Chunk* ch = world.chunk(hoverHit.chunkCoord);
        if(ch) {
            const Tile& t = ch->tile(hoverHit.localTileCoord.x(), hoverHit.localTileCoord.y());
            Vector3 chunkPos{
                Float(hoverHit.chunkCoord.x()) * chunkWorldSize,
                0.0f,
                Float(hoverHit.chunkCoord.y()) * chunkWorldSize
            };
            Vector3 tilePos = chunkPos + Vector3{
                Float(hoverHit.localTileCoord.x()) * world.tileSpacing(),
                0.0f,
                Float(hoverHit.localTileCoord.y()) * world.tileSpacing()
            };

            // Scale to match height and slightly expand for outline overlay
            Matrix4 model = Matrix4::translation(tilePos) *
                            Matrix4::scaling({1.02f, t.height, 1.02f});

            _flatShader.setColor(Color4{1.0f, 0.95f, 0.2f, 0.55f}) // Bright yellow/gold highlight
                       .setTransformationProjectionMatrix(viewProjection * model);
            _flatShader.draw(_highlightMesh);
        }
    }
}

}
