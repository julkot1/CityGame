#pragma once

#include <unordered_set>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Platform/Sdl2Application.h>

namespace Magnum {

class Input {
    public:
        using Key = Platform::Application::Key;
        using Pointer = Platform::Application::Pointer;

        // Forward application events
        void keyPressEvent(Platform::Application::KeyEvent& event);
        void keyReleaseEvent(Platform::Application::KeyEvent& event);
        void pointerPressEvent(Platform::Application::PointerEvent& event);
        void pointerReleaseEvent(Platform::Application::PointerEvent& event);
        void pointerMoveEvent(Platform::Application::PointerMoveEvent& event);
        void scrollEvent(Platform::Application::ScrollEvent& event);

        // Reset per-frame deltas (call once at the end of drawEvent)
        void nextFrame();

        // Keyboard state
        bool isKeyPressed(Key key) const;
        bool isKeyJustPressed(Key key) const;
        bool isKeyJustReleased(Key key) const;

        // Mouse / Pointer button state
        bool isPointerPressed(Pointer pointer) const;
        bool isPointerJustPressed(Pointer pointer) const;
        bool isPointerJustReleased(Pointer pointer) const;

        // Mouse positions and deltas
        Vector2 cursorPosition() const { return _cursorPosition; }
        Vector2 cursorDelta() const { return _cursorDelta; }
        Vector2 scrollDelta() const { return _scrollDelta; }

    private:
        struct EnumClassHash {
            template <typename T>
            std::size_t operator()(T t) const {
                return static_cast<std::size_t>(t);
            }
        };

        std::unordered_set<Key, EnumClassHash> _pressedKeys;
        std::unordered_set<Key, EnumClassHash> _justPressedKeys;
        std::unordered_set<Key, EnumClassHash> _justReleasedKeys;

        std::unordered_set<Pointer, EnumClassHash> _pressedPointers;
        std::unordered_set<Pointer, EnumClassHash> _justPressedPointers;
        std::unordered_set<Pointer, EnumClassHash> _justReleasedPointers;

        Vector2 _cursorPosition{0.0f};
        Vector2 _cursorDelta{0.0f};
        Vector2 _scrollDelta{0.0f};
};

}
