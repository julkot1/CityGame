#include "rendering/Renderer.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Functions.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/ArrayViewStl.h>
#include <vector>
#include <cmath>

namespace Magnum {

using namespace Math::Literals;

Renderer::Renderer() {
    GL::Renderer::setClearColor(0xa5c9ea_rgbf);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
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

    initCloudMesh();
}

void Renderer::initCloudMesh() {
    // Build a multi-tiered 3D voxel cloud cluster using ChunkVertex format so it receives full sunlight & fog
    std::vector<ChunkVertex> vertices;
    std::vector<UnsignedInt> indices;

    auto addCloudBox = [&](const Vector3& c0, const Vector3& c1, Float ao) {
        const Color3 cloudCol{0.92f, 0.90f, 0.86f}; // Warm Victorian parchment-white cloud
        struct Face { Vector3 n, v0, v1, v2, v3; };
        Face faces[6] = {
            {{ 0, 1, 0}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
            {{ 0,-1, 0}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}},
            {{ 0, 0, 1}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
            {{ 0, 0,-1}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}},
            {{-1, 0, 0}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c0.z()}},
            {{ 1, 0, 0}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c1.z()}}
        };
        for(const auto& f : faces) {
            UnsignedInt b = vertices.size();
            Float faceAO = (f.n.y() < 0.0f) ? ao * 0.80f : ao;
            vertices.push_back({f.v0, f.n, cloudCol, faceAO});
            vertices.push_back({f.v1, f.n, cloudCol, faceAO});
            vertices.push_back({f.v2, f.n, cloudCol, faceAO});
            vertices.push_back({f.v3, f.n, cloudCol, faceAO});
            indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 1);
            indices.push_back(b + 0); indices.push_back(b + 3); indices.push_back(b + 2);
        }
    };

    // Main wide cloud belly + upper cumulus puffs + side wings
    addCloudBox({-3.2f, 0.0f, -1.8f}, { 3.2f, 0.9f,  1.8f}, 0.90f);
    addCloudBox({-2.0f, 0.8f, -1.3f}, { 1.6f, 1.8f,  1.3f}, 0.96f);
    addCloudBox({-0.6f, 1.5f, -0.9f}, { 2.2f, 2.3f,  0.9f}, 1.00f);
    addCloudBox({-4.4f, 0.1f, -1.1f}, {-2.4f, 0.8f,  1.2f}, 0.88f);
    addCloudBox({ 2.2f, 0.2f, -1.2f}, { 4.2f, 1.0f,  1.0f}, 0.92f);

    _cloudVertexBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
    _cloudVertexBuffer.setData(Containers::arrayView(vertices), GL::BufferUsage::StaticDraw);

    _cloudIndexBuffer = GL::Buffer{GL::Buffer::TargetHint::ElementArray};
    _cloudIndexBuffer.setData(Containers::arrayView(indices), GL::BufferUsage::StaticDraw);

    _cloudMesh = GL::Mesh{};
    _cloudMesh.setCount(indices.size())
              .addVertexBuffer(_cloudVertexBuffer, 0,
                  GL::Attribute<0, Vector3>{},
                  GL::Attribute<1, Vector3>{},
                  GL::Attribute<2, Color3>{},
                  GL::Attribute<3, Float>{})
              .setIndexBuffer(_cloudIndexBuffer, 0, GL::MeshIndexType::UnsignedInt);
}

void Renderer::renderDriftingClouds(const World& world, const Matrix4&) {
    if(world.chunks().empty()) return;

    const Float minX = world.minWorldX() - 20.0f;
    const Float maxX = world.maxWorldX() + 20.0f;
    const Float minZ = world.minWorldZ() - 20.0f;
    const Float maxZ = world.maxWorldZ() + 20.0f;
    const Float spanX = Math::max(40.0f, maxX - minX);
    const Float spanZ = Math::max(40.0f, maxZ - minZ);

    // Render 14 drifting voxel clouds wrapping seamlessly across the sky bounds
    constexpr int cloudCount = 14;
    for(int i = 0; i < cloudCount; ++i) {
        Float seedX = Float((i * 73 + 19) % 97) / 97.0f;
        Float seedZ = Float((i * 41 + 53) % 89) / 89.0f;
        Float speed = 1.1f + Float(i % 4) * 0.35f;
        Float scale = 0.85f + Float(i % 5) * 0.22f;

        Float cx = minX + std::fmod(seedX * spanX + _timeSeconds * speed, spanX);
        Float cz = minZ + std::fmod(seedZ * spanZ + _timeSeconds * speed * 0.35f, spanZ);
        Float cy = 19.5f + Float(i % 3) * 1.4f + std::sin(_timeSeconds * 0.8f + Float(i)) * 0.25f;

        Matrix4 model = Matrix4::translation({cx, cy, cz}) *
                        Matrix4::scaling({scale, scale * 0.85f, scale});
        _shader.setTransformationMatrix(model);
        _shader.draw(_cloudMesh);
    }
}

void Renderer::setViewport(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
}

void Renderer::render(const World& world, const Camera& camera,
                      const World::RaycastHit& hoverHit,
                      const World::RaycastHit& selectedHit) {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    const Matrix4 viewProjection = camera.viewProjectionMatrix();
    const Float chunkWorldSize = world.chunkSizeInWorldUnits();

    _shader.setProjectionMatrix(viewProjection)
           .setCameraPosition(camera.eyePosition());

    // Extract 6 view frustum planes from viewProjection matrix for Frustum Culling
    // Plane: ax + by + cz + d = 0
    std::array<Vector4, 6> planes;
    // Left
    planes[0] = viewProjection.row(3) + viewProjection.row(0);
    // Right
    planes[1] = viewProjection.row(3) - viewProjection.row(0);
    // Bottom
    planes[2] = viewProjection.row(3) + viewProjection.row(1);
    // Top
    planes[3] = viewProjection.row(3) - viewProjection.row(1);
    // Near
    planes[4] = viewProjection.row(3) + viewProjection.row(2);
    // Far
    planes[5] = viewProjection.row(3) - viewProjection.row(2);

    // Normalize planes
    for(auto& p : planes) {
        Float len = p.xyz().length();
        if(len > 1e-6f) p /= len;
    }

    _lastDrawnChunks = 0;

    // Render visible chunks only
    for(const auto& entry : world.chunks()) {
        const Chunk& chunk = entry.second;
        if(!chunk.hasMesh()) continue;

        Vector3 minPos{
            Float(chunk.chunkCoord().x()) * chunkWorldSize,
            0.0f,
            Float(chunk.chunkCoord().y()) * chunkWorldSize
        };
        Vector3 maxPos{
            minPos.x() + chunkWorldSize,
            25.0f, // Max possible world height
            minPos.z() + chunkWorldSize
        };

        // AABB vs Frustum culling test
        bool outside = false;
        for(const auto& plane : planes) {
            // Find positive vertex along plane normal
            Vector3 pVertex{
                plane.x() > 0.0f ? maxPos.x() : minPos.x(),
                plane.y() > 0.0f ? maxPos.y() : minPos.y(),
                plane.z() > 0.0f ? maxPos.z() : minPos.z()
            };

            if(Math::dot(plane.xyz(), pVertex) + plane.w() < 0.0f) {
                outside = true;
                break;
            }
        }

        if(outside) continue; // Culled! Skip drawing this chunk completely

        Matrix4 modelMatrix = Matrix4::translation(minPos);
        _shader.setTransformationMatrix(modelMatrix);
        _shader.draw(chunk.mesh());
        _lastDrawnChunks++;
    }

    // Render drifting 3D voxel clouds high in the sky
    renderDriftingClouds(world, viewProjection);

    // Smooth breathing/pulsing animation for selection and hover indicators
    Float pulse = 0.5f + 0.5f * std::sin(_timeSeconds * 4.5f);
    Float bobOffset = std::sin(_timeSeconds * 3.5f) * 0.025f;

    // Render selected tile overlay (animated cyan outline box)
    if(selectedHit.hit) {
        const Chunk* ch = world.chunk(selectedHit.chunkCoord);
        if(ch) {
            const Tile& t = ch->tile(selectedHit.localTileCoord.x(), selectedHit.localTileCoord.y());
            Vector3 chunkPos{
                Float(selectedHit.chunkCoord.x()) * chunkWorldSize,
                0.0f,
                Float(selectedHit.chunkCoord.y()) * chunkWorldSize
            };
            Vector3 tilePos = chunkPos + Vector3{
                Float(selectedHit.localTileCoord.x()) * world.tileSpacing(),
                bobOffset,
                Float(selectedHit.localTileCoord.y()) * world.tileSpacing()
            };

            Float scaleXZ = 1.025f + pulse * 0.02f;
            Matrix4 model = Matrix4::translation(tilePos) *
                            Matrix4::scaling({scaleXZ, t.height * 1.03f, scaleXZ});

            _flatShader.setColor(Color4{0.2f, 0.85f, 1.0f, 0.45f + pulse * 0.25f}) // Animated cyan selection
                       .setTransformationProjectionMatrix(viewProjection * model);
            _flatShader.draw(_highlightMesh);
        }
    }

    // Render hover highlight overlay if hovering a valid tile (and not already selected)
    if(hoverHit.hit && (!selectedHit.hit || hoverHit.worldTileX != selectedHit.worldTileX || hoverHit.worldTileZ != selectedHit.worldTileZ)) {
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
                bobOffset * 0.6f,
                Float(hoverHit.localTileCoord.y()) * world.tileSpacing()
            };

            // Scale to match height and slightly expand for outline overlay
            Float scaleXZ = 1.015f + pulse * 0.015f;
            Matrix4 model = Matrix4::translation(tilePos) *
                            Matrix4::scaling({scaleXZ, t.height * 1.02f, scaleXZ});

            _flatShader.setColor(Color4{1.0f, 0.90f, 0.25f, 0.38f + pulse * 0.20f}) // Warm pulsing brass highlight
                       .setTransformationProjectionMatrix(viewProjection * model);
            _flatShader.draw(_highlightMesh);
        }
    }
}

}
