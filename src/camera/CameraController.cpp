#include "camera/CameraController.h"

namespace Magnum {

using namespace Math::Literals;

CameraController::CameraController(Camera& camera, Float panSpeed, Float rotateSpeed):
    _camera{camera},
    _panSpeed{panSpeed},
    _rotateSpeed{rotateSpeed}
{}

void CameraController::update(const Input& input, Float dt) {
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

    // Pan with WASD / Arrow keys relative to camera heading
    Vector3 move{0.0f};
    if(input.isKeyPressed(Input::Key::W) || input.isKeyPressed(Input::Key::Up))    move += _camera.forward();
    if(input.isKeyPressed(Input::Key::S) || input.isKeyPressed(Input::Key::Down))  move -= _camera.forward();
    if(input.isKeyPressed(Input::Key::D) || input.isKeyPressed(Input::Key::Right)) move += _camera.right();
    if(input.isKeyPressed(Input::Key::A) || input.isKeyPressed(Input::Key::Left))  move -= _camera.right();

    if(!move.isZero())
        _camera.move(move.normalized() * _panSpeed * dt);

    // Rotate with Q / E
    if(input.isKeyPressed(Input::Key::Q)) _camera.rotate(Rad{-_rotateSpeed * dt}, 0.0_radf);
    if(input.isKeyPressed(Input::Key::E)) _camera.rotate(Rad{ _rotateSpeed * dt}, 0.0_radf);
}

}
