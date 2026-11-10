#include "camera/Camera.h"
#include "camera/CameraController.h"
#include "input/Input.h"
#include "rendering/Renderer.h"
#include <Corrade/Containers/StringView.h>
#include <Corrade/Utility/Debug.h>
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

        Renderer _renderer;
        Timeline _timeline;
        Camera _camera;
        CameraController _cameraController;
        Input _input;
};

MyApplication::MyApplication(const Arguments& arguments):
    Platform::Application{arguments},
    _camera{framebufferSize()},
    _cameraController{_camera}
{
    _renderer.setViewport(framebufferSize());

    Debug{} << "Hello! This application is running on"
            << GL::Context::current().version() << "using"
            << GL::Context::current().rendererString();

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

    _cameraController.update(_input, dt);
    _renderer.render(_camera);

    swapBuffers();
    _timeline.nextFrame();
    _input.nextFrame();
    redraw();
}

MAGNUM_APPLICATION_MAIN(MyApplication)
