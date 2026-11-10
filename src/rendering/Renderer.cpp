#include "rendering/Renderer.h"

#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

using namespace Math::Literals;

Renderer::Renderer() {
    GL::Renderer::setClearColor(0xa5c9ea_rgbf);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    struct TriangleVertex {
        Vector3 position;
        Color3 color;
    };
    const TriangleVertex vertices[]{
        // Front face - red
        {{-0.5f, -0.5f,  0.5f}, 0xff0000_rgbf},
        {{ 0.5f, -0.5f,  0.5f}, 0xff0000_rgbf},
        {{ 0.5f,  0.5f,  0.5f}, 0xff0000_rgbf},
        {{-0.5f,  0.5f,  0.5f}, 0xff0000_rgbf},

        // Back face - green
        {{-0.5f, -0.5f, -0.5f}, 0x00ff00_rgbf},
        {{ 0.5f, -0.5f, -0.5f}, 0x00ff00_rgbf},
        {{ 0.5f,  0.5f, -0.5f}, 0x00ff00_rgbf},
        {{-0.5f,  0.5f, -0.5f}, 0x00ff00_rgbf}
    };
    const UnsignedInt indices[]{
        // Front
        0, 1, 2,  0, 2, 3,

        // Back
        5, 4, 7,  5, 7, 6,

        // Left
        4, 0, 3,  4, 3, 7,

        // Right
        1, 5, 6,  1, 6, 2,

        // Top
        3, 2, 6,  3, 6, 7,

        // Bottom
        4, 5, 1,  4, 1, 0
    };

    _mesh.setCount(Containers::arraySize(indices))
        .addVertexBuffer(GL::Buffer{vertices}, 0,
            Shaders::VertexColorGL3D::Position{},
            Shaders::VertexColorGL3D::Color3{})
        .setIndexBuffer(GL::Buffer{indices}, 0, GL::MeshIndexType::UnsignedInt);
}

void Renderer::setViewport(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
}

void Renderer::render(const Camera& camera) {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    _shader.setTransformationProjectionMatrix(camera.viewProjectionMatrix());
    _shader.draw(_mesh);
}

}
