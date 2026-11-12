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

namespace {

Vector3 rotateYQuarterTurns(const Vector3& v, UnsignedInt rot) {
    switch(rot & 3u) {
        case 1u: return Vector3{ v.z(), v.y(), -v.x()};
        case 2u: return Vector3{-v.x(), v.y(), -v.z()};
        case 3u: return Vector3{-v.z(), v.y(),  v.x()};
        default: return v;
    }
}

}

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

void Renderer::update(Float dt, const World& world) {
    // 1. Advance existing puffs and remove expired ones
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

    // 2. Spawn new puffs from chimneys periodically
    _smokeSpawnTimer += dt;
    if(_smokeSpawnTimer >= 0.35f) {
        _smokeSpawnTimer = 0.0f;

        const Float tileSpacing = world.tileSpacing();
        for(const auto& pair : world.chunks()) {
            const Chunk& chunk = pair.second;
            const Vector3 chunkOrigin = chunk.worldPosition(tileSpacing);

            for(const Entity& entity : chunk.entities()) {
                Vector3 localChimney;
                bool hasChimney = false;

                if(entity.type == EntityType::House2x2) {
                    localChimney = Vector3{0.55f, 3.12f, -0.27f};
                    hasChimney = true;
                } else if(entity.type == EntityType::HouseStandalone2x2) {
                    localChimney = Vector3{0.57f, 2.42f, -0.22f};
                    hasChimney = true;
                } else if(entity.type == EntityType::CityCenter2x3) {
                    localChimney = Vector3{0.56f, 2.70f, -1.22f};
                    hasChimney = true;
                }

                if(hasChimney && _smokePuffs.size() < 300) {
                    Vector3 chimneyPos = chunkOrigin + entity.position + rotateYQuarterTurns(localChimney, entity.data & 3u);
                    Float seed = Float((_smokePuffs.size() * 37 + std::rand()) % 1000) / 1000.0f;
                    Float size = 0.85f + seed * 0.35f;
                    _smokePuffs.push_back(SmokePuff{chimneyPos, 0.0f, seed, size});
                }
            }
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

        // Smooth quadratic ease-out fade
        Float fade = 1.0f - progress;
        fade = fade * (2.0f - fade);

        // Dense base opacity fading smoothly with height
        Float alpha = 0.82f * fade;

        // Expanding volumetric size
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

void Renderer::render(const World& world, const Camera& camera,
                      const World::RaycastHit& hoverHit,
                      const World::RaycastHit& selectedHit,
                      const Vector2i& hoverFootprint,
                      bool hoverValid) {
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

        Vector3 chunkOrigin{
            Float(chunk.chunkCoord().x()) * chunkWorldSize,
            0.0f,
            Float(chunk.chunkCoord().y()) * chunkWorldSize
        };
        Vector3 minPos = chunkOrigin - Vector3{2.0f, 0.0f, 2.0f};
        Vector3 maxPos = chunkOrigin + Vector3{
            chunkWorldSize + 2.0f,
            25.0f, // Max possible world height
            chunkWorldSize + 2.0f
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

        Matrix4 modelMatrix = Matrix4::translation(chunkOrigin);
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

    // Render hover highlight overlay if hovering a valid tile (supports arbitrary NxM building footprints)
    if(hoverHit.hit && (hoverFootprint.x() > 1 || hoverFootprint.y() > 1 || !selectedHit.hit || hoverHit.worldTileX != selectedHit.worldTileX || hoverHit.worldTileZ != selectedHit.worldTileZ)) {
        const Chunk* ch = world.chunk(hoverHit.chunkCoord);
        if(ch) {
            const Tile& t = ch->tile(hoverHit.localTileCoord.x(), hoverHit.localTileCoord.y());
            Vector3 chunkPos{
                Float(hoverHit.chunkCoord.x()) * chunkWorldSize,
                0.0f,
                Float(hoverHit.chunkCoord.y()) * chunkWorldSize
            };
            Float centerOffsetX = Float(hoverFootprint.x() - 1) * 0.5f * world.tileSpacing();
            Float centerOffsetZ = Float(hoverFootprint.y() - 1) * 0.5f * world.tileSpacing();
            Vector3 tilePos = chunkPos + Vector3{
                Float(hoverHit.localTileCoord.x()) * world.tileSpacing() + centerOffsetX,
                bobOffset * 0.6f,
                Float(hoverHit.localTileCoord.y()) * world.tileSpacing() + centerOffsetZ
            };

            // Scale to match footprint (NxM) and height
            Float scaleX = Float(hoverFootprint.x()) + 0.02f + pulse * 0.015f;
            Float scaleZ = Float(hoverFootprint.y()) + 0.02f + pulse * 0.015f;
            Matrix4 model = Matrix4::translation(tilePos) *
                            Matrix4::scaling({scaleX, t.height * 1.02f, scaleZ});

            Color4 hoverColor = hoverValid
                ? Color4{1.0f, 0.90f, 0.25f, 0.38f + pulse * 0.20f}  // Warm pulsing brass/gold
                : Color4{0.95f, 0.28f, 0.25f, 0.45f + pulse * 0.20f}; // Red invalid placement warning

            _flatShader.setColor(hoverColor)
                       .setTransformationProjectionMatrix(viewProjection * model);
            _flatShader.draw(_highlightMesh);
        }
    }

    // Render soft translucent chimney smoke plumes
    renderSmoke(viewProjection);
}

}
