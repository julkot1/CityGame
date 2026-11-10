#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Math/Vector2.h>
#include "camera/Camera.h"

namespace Magnum {

class Renderer {
    public:
        explicit Renderer();

        void setViewport(const Vector2i& size);
        void render(const Camera& camera);

    private:
        GL::Mesh _mesh;
        Shaders::VertexColorGL3D _shader;
};

}
