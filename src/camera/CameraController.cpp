#include "camera/CameraController.h"
#include <Magnum/Math/Functions.h>

namespace Magnum {

using namespace Math::Literals;

CameraController::CameraController(Camera& camera, Float panSpeed, Float rotateSpeed):
    _camera{camera},
    _panSpeed{panSpeed},
    _rotateSpeed{rotateSpeed}
{}

void CameraController::update(const Input& input, Float dt, const Vector3& minBounds, const Vector3& maxBounds) {
    // Mouse orbit when middle or right mouse button held
    if(input.isPointerPressed(Input::Pointer::MouseMiddle) ||
       input.isPointerPressed(Input::Pointer::MouseRight))
    {
        Vector2 delta = input.cursorDelta();
        _camera.rotate(Rad{-delta.x() * 0.005f}, Rad{delta.y() * 0.005f});
    }

    // Mouse zoom
    if(input.scrollDelta().y() != 0.0f) {
        _camera.zoom(input.scrollDelta().y());
    }

    // Dynamic pan speed: scales with camera distance + 2.5x sprint with Shift
    Float zoomMultiplier = Math::max(0.4f, _camera.distance() * 0.035f);
    Float effectivePanSpeed = _panSpeed * zoomMultiplier;
    Float speed = (input.isKeyPressed(Input::Key::LeftShift) || input.isKeyPressed(Input::Key::RightShift))
        ? effectivePanSpeed * 2.5f : effectivePanSpeed;

    Vector3 move{0.0f};
    if(input.isKeyPressed(Input::Key::W) || input.isKeyPressed(Input::Key::Up))    move += _camera.forward();
    if(input.isKeyPressed(Input::Key::S) || input.isKeyPressed(Input::Key::Down))  move -= _camera.forward();
    if(input.isKeyPressed(Input::Key::D) || input.isKeyPressed(Input::Key::Right)) move += _camera.right();
    if(input.isKeyPressed(Input::Key::A) || input.isKeyPressed(Input::Key::Left))  move -= _camera.right();

    if(!move.isZero()) {
        _camera.move(move.normalized() * speed * dt);
        _camera.clampTarget(minBounds, maxBounds);
    }

    // Rotate with Q / E
    if(input.isKeyPressed(Input::Key::Q)) _camera.rotate(Rad{-_rotateSpeed * dt}, 0.0_radf);
    if(input.isKeyPressed(Input::Key::E)) _camera.rotate(Rad{ _rotateSpeed * dt}, 0.0_radf);
}

}
