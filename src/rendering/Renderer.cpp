#include "rendering/Renderer.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Functions.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/ArrayViewStl.h>
#include <vector>
#include <cmath>
#include <algorithm>

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

void Renderer::setWorldBounds(Float minX, Float maxX, Float minZ, Float maxZ, Float tileSpacing) {
    _minWorldX = minX;
    _maxWorldX = maxX;
    _minWorldZ = minZ;
    _maxWorldZ = maxZ;
    _tileSpacing = tileSpacing;
    _chunkWorldSize = Float(Chunk::Size) * tileSpacing;
}

void Renderer::uploadChunkMesh(ChunkMeshData&& meshData) {
    auto key = std::make_pair(meshData.chunkCoord.x(), meshData.chunkCoord.y());
    RenderChunk& rc = _renderChunks[key];
    rc.chunkCoord = meshData.chunkCoord;

    if(meshData.indices.empty()) {
        rc.hasMesh = false;
        rc.indexCount = 0;
        return;
    }

    rc.vertexBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
    rc.vertexBuffer.setData(Containers::arrayView(meshData.vertices), GL::BufferUsage::StaticDraw);

    rc.indexBuffer = GL::Buffer{GL::Buffer::TargetHint::ElementArray};
    rc.indexBuffer.setData(Containers::arrayView(meshData.indices), GL::BufferUsage::StaticDraw);

    rc.mesh = GL::Mesh{};
    rc.mesh.setCount(meshData.indices.size())
           .addVertexBuffer(rc.vertexBuffer, 0,
              GL::Attribute<0, Vector3>{},
              GL::Attribute<1, Vector3>{},
              GL::Attribute<2, Color3>{},
              GL::Attribute<3, Float>{})
           .setIndexBuffer(rc.indexBuffer, 0, GL::MeshIndexType::UnsignedInt);
    rc.hasMesh = true;
    rc.indexCount = static_cast<UnsignedInt>(meshData.indices.size());
}

void Renderer::clearRenderChunks() {
    _renderChunks.clear();
}

void Renderer::initCloudMesh() {
    std::vector<ChunkVertex> vertices;
    std::vector<UnsignedInt> indices;

    auto addCloudBox = [&](const Vector3& c0, const Vector3& c1, Float ao) {
        const Color3 cloudCol{0.92f, 0.90f, 0.86f};
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

    addCloudBox({-6.5f, 0.0f, -4.5f}, {6.5f, 1.8f, 4.5f}, 0.88f);
    addCloudBox({-4.2f, 1.8f, -2.8f}, {4.2f, 3.2f, 2.8f}, 0.94f);
    addCloudBox({-2.0f, 3.2f, -1.5f}, {2.0f, 4.2f, 1.5f}, 1.00f);

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

void Renderer::renderDriftingClouds(const Matrix4&) {
    if(_renderChunks.empty()) return;

    const Float minX = _minWorldX - 20.0f;
    const Float maxX = _maxWorldX + 20.0f;
    const Float minZ = _minWorldZ - 20.0f;
    const Float maxZ = _maxWorldZ + 20.0f;
    const Float spanX = Math::max(40.0f, maxX - minX);
    const Float spanZ = Math::max(40.0f, maxZ - minZ);

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

void Renderer::update(Float dt, const std::vector<Vector3>& chimneyPositions) {
    for(auto& p : _smokePuffs) {
        p.age += dt;
        Float wobble = std::sin(p.age * 2.5f + p.seed * 6.28f) * 0.05f;
        p.pos += Vector3{(0.18f + wobble) * dt, 0.60f * dt, (0.08f + wobble * 0.5f) * dt};
    }
    _smokePuffs.erase(
        std::remove_if(_smokePuffs.begin(), _smokePuffs.end(), [](const SmokePuff& p) {
            return p.age >= 2.8f;
        }),
        _smokePuffs.end()
    );

    _smokeSpawnTimer += dt;
    if(_smokeSpawnTimer >= 0.35f && !chimneyPositions.empty()) {
        _smokeSpawnTimer = 0.0f;
        for(const Vector3& chimneyPos : chimneyPositions) {
            if(_smokePuffs.size() >= 300) break;
            Float seed = Float((_smokePuffs.size() * 37 + std::rand()) % 1000) / 1000.0f;
            Float size = 0.85f + seed * 0.35f;
            _smokePuffs.push_back(SmokePuff{chimneyPos, 0.0f, seed, size});
        }
    }
}

void Renderer::renderSmoke(const Matrix4& viewProjection) {
    if(_smokePuffs.empty()) return;

    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(
        GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha
    );

    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::setDepthMask(false);

    for(const auto& p : _smokePuffs) {
        Float progress = Math::clamp(p.age / 2.8f, 0.0f, 1.0f);
        Float fade = 1.0f - progress;
        fade = fade * (2.0f - fade);
        Float alpha = 0.82f * fade;
        Float scale = (0.14f + progress * 0.35f) * p.size;

        Matrix4 model = Matrix4::translation(p.pos - Vector3{0.0f, scale * 0.5f, 0.0f})
                      * Matrix4::scaling(Vector3{scale, scale * 1.15f, scale});

        _flatShader.setColor(Color4{0.72f, 0.71f, 0.69f, alpha})
                   .setTransformationProjectionMatrix(viewProjection * model);
        _flatShader.draw(_highlightMesh);
    }

    GL::Renderer::setDepthMask(true);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
}

void Renderer::setViewport(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
}

void Renderer::render(const Camera& camera,
                      const World::RaycastHit& hoverHit,
                      const World::RaycastHit& selectedHit,
                      const Vector2i& hoverFootprint,
                      bool hoverValid) {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    const Matrix4 viewProjection = camera.viewProjectionMatrix();
    const Float chunkWorldSize = _chunkWorldSize;

    _shader.setProjectionMatrix(viewProjection)
           .setCameraPosition(camera.eyePosition());

    // Extract 6 view frustum planes from viewProjection matrix for Frustum Culling
    std::array<Vector4, 6> planes;
    planes[0] = viewProjection.row(3) + viewProjection.row(0); // Left
    planes[1] = viewProjection.row(3) - viewProjection.row(0); // Right
    planes[2] = viewProjection.row(3) + viewProjection.row(1); // Bottom
    planes[3] = viewProjection.row(3) - viewProjection.row(1); // Top
    planes[4] = viewProjection.row(3) + viewProjection.row(2); // Near
    planes[5] = viewProjection.row(3) - viewProjection.row(2); // Far

    for(auto& p : planes) {
        Float len = p.xyz().length();
        if(len > 1e-6f) p /= len;
    }

    _lastDrawnChunks = 0;

    // Render visible chunks only
    for(auto& entry : _renderChunks) {
        RenderChunk& rc = entry.second;
        if(!rc.hasMesh) continue;

        Vector3 chunkOrigin{
            Float(rc.chunkCoord.x()) * chunkWorldSize,
            0.0f,
            Float(rc.chunkCoord.y()) * chunkWorldSize
        };
        Vector3 minPos = chunkOrigin - Vector3{2.0f, 0.0f, 2.0f};
        Vector3 maxPos = chunkOrigin + Vector3{
            chunkWorldSize + 2.0f,
            25.0f,
            chunkWorldSize + 2.0f
        };

        bool outside = false;
        for(const auto& plane : planes) {
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

        if(outside) continue;

        Matrix4 modelMatrix = Matrix4::translation(chunkOrigin);
        _shader.setTransformationMatrix(modelMatrix);
        _shader.draw(rc.mesh);
        _lastDrawnChunks++;
    }

    renderDriftingClouds(viewProjection);

    Float pulse = 0.5f + 0.5f * std::sin(_timeSeconds * 4.5f);
    Float bobOffset = std::sin(_timeSeconds * 3.5f) * 0.025f;

    // Render selected tile overlay (animated cyan box)
    if(selectedHit.hit) {
        Vector3 chunkPos{
            Float(selectedHit.chunkCoord.x()) * chunkWorldSize,
            0.0f,
            Float(selectedHit.chunkCoord.y()) * chunkWorldSize
        };
        Vector3 tilePos = chunkPos + Vector3{
            Float(selectedHit.localTileCoord.x()) * _tileSpacing,
            bobOffset,
            Float(selectedHit.localTileCoord.y()) * _tileSpacing
        };

        Float scaleXZ = 1.025f + pulse * 0.02f;
        Float height = Math::max(0.2f, selectedHit.tileHeight);
        Matrix4 model = Matrix4::translation(tilePos) *
                        Matrix4::scaling({scaleXZ, height * 1.03f, scaleXZ});

        _flatShader.setColor(Color4{0.2f, 0.85f, 1.0f, 0.45f + pulse * 0.25f})
                   .setTransformationProjectionMatrix(viewProjection * model);
        _flatShader.draw(_highlightMesh);
    }

    // Render hover highlight overlay
    if(hoverHit.hit && (hoverFootprint.x() > 1 || hoverFootprint.y() > 1 || !selectedHit.hit || hoverHit.worldTileX != selectedHit.worldTileX || hoverHit.worldTileZ != selectedHit.worldTileZ)) {
        Vector3 chunkPos{
            Float(hoverHit.chunkCoord.x()) * chunkWorldSize,
            0.0f,
            Float(hoverHit.chunkCoord.y()) * chunkWorldSize
        };
        Float centerOffsetX = Float(hoverFootprint.x() - 1) * 0.5f * _tileSpacing;
        Float centerOffsetZ = Float(hoverFootprint.y() - 1) * 0.5f * _tileSpacing;
        Vector3 tilePos = chunkPos + Vector3{
            Float(hoverHit.localTileCoord.x()) * _tileSpacing + centerOffsetX,
            bobOffset * 0.6f,
            Float(hoverHit.localTileCoord.y()) * _tileSpacing + centerOffsetZ
        };

        Float scaleX = Float(hoverFootprint.x()) + 0.02f + pulse * 0.015f;
        Float scaleZ = Float(hoverFootprint.y()) + 0.02f + pulse * 0.015f;
        Float height = Math::max(0.2f, hoverHit.tileHeight);
        Matrix4 model = Matrix4::translation(tilePos) *
                        Matrix4::scaling({scaleX, height * 1.02f, scaleZ});

        Color4 hoverColor = hoverValid
            ? Color4{1.0f, 0.90f, 0.25f, 0.38f + pulse * 0.20f}
            : Color4{0.95f, 0.28f, 0.25f, 0.45f + pulse * 0.20f};

        _flatShader.setColor(hoverColor)
                   .setTransformationProjectionMatrix(viewProjection * model);
        _flatShader.draw(_highlightMesh);
    }

    renderSmoke(viewProjection);
}

}
