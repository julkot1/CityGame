#pragma once

#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Math/Vector2.h>
#include "camera/Camera.h"
#include "world/World.h"

namespace Magnum {

class Renderer {
    public:
        explicit Renderer();

        void setViewport(const Vector2i& size);
        void render(const World& world, const Camera& camera, const World::RaycastHit& hoverHit = {});

    private:
        Shaders::VertexColorGL3D _shader;
        Shaders::FlatGL3D _flatShader;
        GL::Mesh _highlightMesh;
        GL::Buffer _highlightVertexBuffer{NoCreate};
        GL::Buffer _highlightIndexBuffer{NoCreate};
};

}
