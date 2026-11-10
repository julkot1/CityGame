#pragma once

#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Math/Vector2.h>
#include "camera/Camera.h"
#include "world/World.h"

namespace Magnum {

class Renderer {
    public:
        explicit Renderer();

        void setViewport(const Vector2i& size);
        void render(const World& world, const Camera& camera);

    private:
        Shaders::VertexColorGL3D _shader;
};

}
