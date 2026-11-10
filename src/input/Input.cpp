#include "input/Input.h"

namespace Magnum {

void Input::keyPressEvent(Platform::Application::KeyEvent& event) {
    if(!event.isRepeated()) {
        _pressedKeys.insert(event.key());
        _justPressedKeys.insert(event.key());
    }
}

void Input::keyReleaseEvent(Platform::Application::KeyEvent& event) {
    _pressedKeys.erase(event.key());
    _justReleasedKeys.insert(event.key());
}

void Input::pointerPressEvent(Platform::Application::PointerEvent& event) {
    _pressedPointers.insert(event.pointer());
    _justPressedPointers.insert(event.pointer());
    _cursorPosition = event.position();
}

void Input::pointerReleaseEvent(Platform::Application::PointerEvent& event) {
    _pressedPointers.erase(event.pointer());
    _justReleasedPointers.insert(event.pointer());
    _cursorPosition = event.position();
}

void Input::pointerMoveEvent(Platform::Application::PointerMoveEvent& event) {
    _cursorPosition = event.position();
    _cursorDelta += event.relativePosition();
}

void Input::scrollEvent(Platform::Application::ScrollEvent& event) {
    _scrollDelta += event.offset();
}

void Input::nextFrame() {
    _justPressedKeys.clear();
    _justReleasedKeys.clear();
    _justPressedPointers.clear();
    _justReleasedPointers.clear();
    _cursorDelta = Vector2{0.0f};
    _scrollDelta = Vector2{0.0f};
}

bool Input::isKeyPressed(Key key) const {
    return _pressedKeys.find(key) != _pressedKeys.end();
}

bool Input::isKeyJustPressed(Key key) const {
    return _justPressedKeys.find(key) != _justPressedKeys.end();
}

bool Input::isKeyJustReleased(Key key) const {
    return _justReleasedKeys.find(key) != _justReleasedKeys.end();
}

bool Input::isPointerPressed(Pointer pointer) const {
    return _pressedPointers.find(pointer) != _pressedPointers.end();
}

bool Input::isPointerJustPressed(Pointer pointer) const {
    return _justPressedPointers.find(pointer) != _justPressedPointers.end();
}

bool Input::isPointerJustReleased(Pointer pointer) const {
    return _justReleasedPointers.find(pointer) != _justReleasedPointers.end();
}

}
