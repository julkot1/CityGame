#pragma once

#include <vector>
#include <map>
#include <utility>
#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include "camera/Camera.h"
#include "world/World.h"
#include "world/ChunkMeshData.h"
#include "rendering/RenderChunk.h"
#include "rendering/TerrainShader.h"

namespace Magnum {

struct SmokePuff {
    Vector3 pos;
    Float age{0.0f};
    Float seed{0.0f};
    Float size{1.0f};
};

class Renderer {
    public:
        explicit Renderer();

        void setViewport(const Vector2i& size);
        void setTime(Float timeSeconds) { _timeSeconds = timeSeconds; _shader.setTime(timeSeconds); }
        void setLightColor(const Color3& color) { _lightColor = color; _shader.setLightColor(color); }
        void setAmbientColor(const Color3& color) { _ambientColor = color; _shader.setAmbientColor(color); }

        void setWorldBounds(Float minX, Float maxX, Float minZ, Float maxZ, Float tileSpacing = 1.0f);
        void uploadChunkMesh(ChunkMeshData&& meshData);
        void clearRenderChunks();

        void update(Float dt, const std::vector<Vector3>& chimneyPositions = {});

        void render(const Camera& camera,
                    const World::RaycastHit& hoverHit = {},
                    const World::RaycastHit& selectedHit = {},
                    const Vector2i& hoverFootprint = Vector2i{1, 1},
                    bool hoverValid = true);

        // Backwards compatibility overload
        void render(const World&, const Camera& camera,
                    const World::RaycastHit& hoverHit = {},
                    const World::RaycastHit& selectedHit = {},
                    const Vector2i& hoverFootprint = Vector2i{1, 1},
                    bool hoverValid = true) {
            render(camera, hoverHit, selectedHit, hoverFootprint, hoverValid);
        }

        UnsignedInt lastDrawnChunks() const { return _lastDrawnChunks; }
        UnsignedInt totalRenderChunks() const { return static_cast<UnsignedInt>(_renderChunks.size()); }

        TerrainShader& terrainShader() { return _shader; }

    private:
        void initCloudMesh();
        void renderDriftingClouds(const Matrix4& viewProjection);
        void renderSmoke(const Matrix4& viewProjection);

        UnsignedInt _lastDrawnChunks{0};
        Float _timeSeconds{0.0f};
        Color3 _lightColor{1.0f, 0.92f, 0.78f};
        Color3 _ambientColor{0.20f, 0.26f, 0.38f};

        Float _minWorldX{-128.0f};
        Float _maxWorldX{128.0f};
        Float _minWorldZ{-128.0f};
        Float _maxWorldZ{128.0f};
        Float _tileSpacing{1.0f};
        Float _chunkWorldSize{32.0f};

        TerrainShader _shader;
        Shaders::FlatGL3D _flatShader;
        GL::Mesh _highlightMesh;
        GL::Buffer _highlightVertexBuffer{NoCreate};
        GL::Buffer _highlightIndexBuffer{NoCreate};

        GL::Mesh _cloudMesh{NoCreate};
        GL::Buffer _cloudVertexBuffer{NoCreate};
        GL::Buffer _cloudIndexBuffer{NoCreate};

        std::map<std::pair<Int, Int>, RenderChunk> _renderChunks;

        std::vector<SmokePuff> _smokePuffs;
        Float _smokeSpawnTimer{0.0f};
};

}
