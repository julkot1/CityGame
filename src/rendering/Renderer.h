#pragma once

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include "camera/Camera.h"
#include "world/World.h"
#include "rendering/TerrainShader.h"

namespace Magnum {

class Renderer {
    public:
        explicit Renderer();

        void setViewport(const Vector2i& size);
        void setTime(Float timeSeconds) { _timeSeconds = timeSeconds; _shader.setTime(timeSeconds); }
        void render(const World& world, const Camera& camera,
                    const World::RaycastHit& hoverHit = {},
                    const World::RaycastHit& selectedHit = {});

        UnsignedInt lastDrawnChunks() const { return _lastDrawnChunks; }

        TerrainShader& terrainShader() { return _shader; }

    private:
        void initCloudMesh();
        void renderDriftingClouds(const World& world, const Matrix4& viewProjection);

        UnsignedInt _lastDrawnChunks{0};
        Float _timeSeconds{0.0f};
        TerrainShader _shader;
        Shaders::FlatGL3D _flatShader;
        GL::Mesh _highlightMesh;
        GL::Buffer _highlightVertexBuffer{NoCreate};
        GL::Buffer _highlightIndexBuffer{NoCreate};

        GL::Mesh _cloudMesh{NoCreate};
        GL::Buffer _cloudVertexBuffer{NoCreate};
        GL::Buffer _cloudIndexBuffer{NoCreate};
};

}
