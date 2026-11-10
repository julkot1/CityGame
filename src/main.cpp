#include "camera/Camera.h"
#include "camera/CameraController.h"
#include "input/Input.h"
#include "rendering/Renderer.h"
#include "world/World.h"
#include "world/WorldInputHandler.h"
#include <Corrade/Containers/StringView.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/FormatStl.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Timeline.h>

using namespace Magnum;

class MyApplication: public Platform::Application {
    public:
        explicit MyApplication(const Arguments& arguments);

    private:
        void viewportEvent(ViewportEvent& event) override;
        void drawEvent() override;
        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent& event) override;
        void pointerPressEvent(PointerEvent& event) override;
        void pointerReleaseEvent(PointerEvent& event) override;
        void pointerMoveEvent(PointerMoveEvent& event) override;
        void scrollEvent(ScrollEvent& event) override;

        World _world{1.0f};
        Renderer _renderer;
        Timeline _timeline;
        Camera _camera;
        CameraController _cameraController;
        Input _input;
        WorldInputHandler _worldInputHandler{_world, _camera};

        UnsignedInt _fpsFrameCount{0};
        Float _fpsTimeAccumulator{0.0f};
};

MyApplication::MyApplication(const Arguments& arguments):
    Platform::Application{arguments},
    _camera{framebufferSize()},
    _cameraController{_camera}
{
    _renderer.setViewport(framebufferSize());

    Debug{} << "CityBuilder Fixed-Size World Initialized!";
    Debug{} << "World Dimensions:" << World::WorldChunkWidth << "x" << World::WorldChunkHeight << "chunks ("
            << (World::WorldChunkWidth * Chunk::Size) << "x" << (World::WorldChunkHeight * Chunk::Size) << "tiles)";
    Debug{} << "Total Chunks:" << _world.chunks().size();
    Debug{} << "Camera bounded within world bounds ["
            << _world.minWorldX() << "," << _world.minWorldZ() << "] to ["
            << _world.maxWorldX() << "," << _world.maxWorldZ() << "]";

    _timeline.start();
}

void MyApplication::viewportEvent(ViewportEvent& event) {
    _renderer.setViewport(event.framebufferSize());
    _camera.setViewport(event.framebufferSize());
}

void MyApplication::keyPressEvent(KeyEvent& event) {
    _input.keyPressEvent(event);
}

void MyApplication::keyReleaseEvent(KeyEvent& event) {
    _input.keyReleaseEvent(event);
}

void MyApplication::pointerPressEvent(PointerEvent& event) {
    _input.pointerPressEvent(event);
}

void MyApplication::pointerReleaseEvent(PointerEvent& event) {
    _input.pointerReleaseEvent(event);
}

void MyApplication::pointerMoveEvent(PointerMoveEvent& event) {
    _input.pointerMoveEvent(event);
}

void MyApplication::scrollEvent(ScrollEvent& event) {
    _input.scrollEvent(event);
}

void MyApplication::drawEvent() {
    Float dt = _timeline.previousFrameDuration();

    // Restrict camera movement strictly within world boundaries
    const Vector3 minBounds{_world.minWorldX(), 0.0f, _world.minWorldZ()};
    const Vector3 maxBounds{_world.maxWorldX(), 0.0f, _world.maxWorldZ()};
    _cameraController.update(_input, dt, minBounds, maxBounds);

    // Update world input (raycasting, hover state, left/right clicks)
    _worldInputHandler.update(_input, framebufferSize());

    _renderer.render(_world, _camera, _worldInputHandler.hoverHit());

    _fpsFrameCount++;
    _fpsTimeAccumulator += dt;
    if(_fpsTimeAccumulator >= 1.0f) {
        Float fps = Float(_fpsFrameCount) / _fpsTimeAccumulator;
        Float frameTimeMs = (_fpsTimeAccumulator / Float(_fpsFrameCount)) * 1000.0f;
        Debug{} << "FPS:" << fps << "| Frame time:" << frameTimeMs << "ms | Chunks:" << _world.chunks().size();

        setWindowTitle(Utility::formatString("City Builder - {:.1f} FPS ({:.2f} ms)", fps, frameTimeMs));

        _fpsFrameCount = 0;
        _fpsTimeAccumulator = 0.0f;
    }

    swapBuffers();
    _timeline.nextFrame();
    _input.nextFrame();
    redraw();
}

MAGNUM_APPLICATION_MAIN(MyApplication)
