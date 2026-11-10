#include "camera/Camera.h"
#include "camera/CameraController.h"
#include "input/Input.h"
#include "rendering/Renderer.h"
#include "world/World.h"
#include "world/WorldInputHandler.h"
#include "world/WorldStorage.h"
#include "world/RandomWorldGenerator.h"
#include "ui/MenuManager.h"
#include "ui/GameHUD.h"
#include <Corrade/Containers/StringView.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/FormatStl.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Timeline.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <imgui.h>

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
        void textInputEvent(TextInputEvent& event) override;

        MenuManager _menuManager;
        GameHUD _gameHUD;

        World _world{1.0f};
        Renderer _renderer;
        Timeline _timeline;
        Camera _camera;
        CameraController _cameraController;
        Input _input;
        WorldInputHandler _worldInputHandler{_world, _camera};

        ImGuiIntegration::Context _imgui{NoCreate};

        UnsignedInt _fpsFrameCount{0};
        Float _fpsTimeAccumulator{0.0f};
};

MyApplication::MyApplication(const Arguments& arguments):
    Platform::Application{arguments, Configuration{}.setTitle("City Builder").setSize({1280, 720})},
    _camera{framebufferSize()},
    _cameraController{_camera}
{
    // Enable VSync (1 = Sync with monitor refresh rate, e.g. 60Hz/144Hz)
    setSwapInterval(1);

    _renderer.setViewport(framebufferSize());

    _imgui = ImGuiIntegration::Context(Vector2{windowSize()}/dpiScaling(),
                                       windowSize(), framebufferSize());
    
    _menuManager.applyCustomTheme();

    _menuManager.setOnCreateWorld([this](const WorldCreationParams& params) {
        auto randGen = dynamic_cast<RandomWorldGenerator*>(_world.generator());
        if(randGen) {
            randGen->setSeed(params.seed);
            randGen->setSeaLevel(params.seaLevel);
        }

        _world.init(params.worldWidthChunks, params.worldHeightChunks);
        WorldStorage::saveToFile(_world, "world.save");

        _camera.setTarget(Vector3{0.0f});
    });
    _menuManager.setOnSaveWorld([this]() {
        WorldStorage::saveToFile(_world, "world.save");
    });
    _menuManager.setOnExit([this]() {
        exit();
    });

    Debug{} << "CityBuilder Initialized!";
    _timeline.start();
}

void MyApplication::viewportEvent(ViewportEvent& event) {
    _renderer.setViewport(event.framebufferSize());
    _camera.setViewport(event.framebufferSize());
    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
                    event.windowSize(), event.framebufferSize());
}

void MyApplication::keyPressEvent(KeyEvent& event) {
    if(_imgui.handleKeyPressEvent(event)) return;

    if(event.key() == KeyEvent::Key::Esc) {
        _menuManager.toggleMenu();
        event.setAccepted();
        return;
    }

    _input.keyPressEvent(event);
}

void MyApplication::keyReleaseEvent(KeyEvent& event) {
    if(_imgui.handleKeyReleaseEvent(event)) return;
    _input.keyReleaseEvent(event);
}

void MyApplication::pointerPressEvent(PointerEvent& event) {
    if(_imgui.handlePointerPressEvent(event)) return;
    _input.pointerPressEvent(event);
}

void MyApplication::pointerReleaseEvent(PointerEvent& event) {
    if(_imgui.handlePointerReleaseEvent(event)) return;
    _input.pointerReleaseEvent(event);
}

void MyApplication::pointerMoveEvent(PointerMoveEvent& event) {
    if(_imgui.handlePointerMoveEvent(event)) return;
    _input.pointerMoveEvent(event);
}

void MyApplication::scrollEvent(ScrollEvent& event) {
    if(_imgui.handleScrollEvent(event)) return;
    _input.scrollEvent(event);
}

void MyApplication::textInputEvent(TextInputEvent& event) {
    if(_imgui.handleTextInputEvent(event)) return;
}

void MyApplication::drawEvent() {
    Float dt = _timeline.previousFrameDuration();

    _imgui.newFrame();

    // Enable OS text input subsystem when ImGui text fields request it
    if(ImGui::GetIO().WantTextInput) {
        startTextInput();
    } else {
        stopTextInput();
    }

    if(_menuManager.isMenuOpen()) {
        // Slowly rotate camera around world in background for cinematic panorama
        _camera.rotate(Rad{0.08f * dt}, 0.0_radf);

        // Render world in background first
        _renderer.render(_world, _camera);

        // Draw menu UI through MenuManager
        _menuManager.draw(windowSize());
    } else {
        // Playing state: update camera and world inputs
        const Vector3 minBounds{_world.minWorldX(), 0.0f, _world.minWorldZ()};
        const Vector3 maxBounds{_world.maxWorldX(), 0.0f, _world.maxWorldZ()};
        _cameraController.update(_input, dt, minBounds, maxBounds);

        // Update world input (raycasting, hover state, left/right clicks)
        _worldInputHandler.update(_input, framebufferSize());

        _renderer.render(_world, _camera, _worldInputHandler.hoverHit());

        // In-game HUD: displays rendered vs culled chunk count
        _gameHUD.update(dt);
        _gameHUD.draw(windowSize(), _renderer.lastDrawnChunks(), _world.chunks().size());
    }

    // Enable/disable appropriate GL states for 2D UI rendering
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

    // Render ImGui overlay on top
    _imgui.drawFrame();

    // Restore 3D states
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    _fpsFrameCount++;
    _fpsTimeAccumulator += dt;
    if(_fpsTimeAccumulator >= 1.0f) {
        Float fps = Float(_fpsFrameCount) / _fpsTimeAccumulator;
        Float frameTimeMs = (_fpsTimeAccumulator / Float(_fpsFrameCount)) * 1000.0f;
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
