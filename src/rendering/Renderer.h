#pragma once

#include <vector>
#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include "camera/Camera.h"
#include "world/World.h"
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
        void update(Float dt, const World& world);

        void setLightColor(const Color3& color) { _lightColor = color; _shader.setLightColor(color); }
        void setAmbientColor(const Color3& color) { _ambientColor = color; _shader.setAmbientColor(color); }

        void render(const World& world, const Camera& camera,
                    const World::RaycastHit& hoverHit = {},
                    const World::RaycastHit& selectedHit = {},
                    const Vector2i& hoverFootprint = Vector2i{1, 1},
                    bool hoverValid = true);

        UnsignedInt lastDrawnChunks() const { return _lastDrawnChunks; }

        TerrainShader& terrainShader() { return _shader; }

    private:
        void initCloudMesh();
        void renderDriftingClouds(const World& world, const Matrix4& viewProjection);
        void renderSmoke(const Matrix4& viewProjection);

        UnsignedInt _lastDrawnChunks{0};
        Float _timeSeconds{0.0f};
        Color3 _lightColor{1.0f, 0.92f, 0.78f};
        Color3 _ambientColor{0.20f, 0.26f, 0.38f};
        TerrainShader _shader;
        Shaders::FlatGL3D _flatShader;
        GL::Mesh _highlightMesh;
        GL::Buffer _highlightVertexBuffer{NoCreate};
        GL::Buffer _highlightIndexBuffer{NoCreate};

        GL::Mesh _cloudMesh{NoCreate};
        GL::Buffer _cloudVertexBuffer{NoCreate};
        GL::Buffer _cloudIndexBuffer{NoCreate};

        std::vector<SmokePuff> _smokePuffs;
        Float _smokeSpawnTimer{0.0f};
};

}
