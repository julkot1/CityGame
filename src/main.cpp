#include "camera/Camera.h"
#include "camera/CameraController.h"
#include "input/Input.h"
#include "rendering/Renderer.h"
#include "world/World.h"
#include "world/TimeManager.h"
#include "world/WorldInputHandler.h"
#include "world/WorldStorage.h"
#include "world/RandomWorldGenerator.h"
#include "building/BuildSystem.h"
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
#include <fstream>

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
        TimeManager _timeManager;
        BuildSystem _buildSystem;

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
        Float _elapsedTime{0.0f};
};

MyApplication::MyApplication(const Arguments& arguments):
    Platform::Application{arguments, Configuration{}.setTitle("City Builder").setSize({1280, 720})},
    _camera{framebufferSize()},
    _cameraController{_camera}
{
    // Enable VSync (1 = Sync with monitor refresh rate, e.g. 60Hz/144Hz)
    setSwapInterval(1);

    _renderer.setViewport(framebufferSize());

    // Initialize ImGui with custom font if available
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    const char* fontPaths[] = {
        "assets/fonts/gomarice_no_continue.ttf",
        "../assets/fonts/gomarice_no_continue.ttf"
    };
    bool fontLoaded = false;
    for(const char* path : fontPaths) {
        if(std::ifstream(path).good()) {
            io.Fonts->AddFontFromFileTTF(path, 16.0f);
            fontLoaded = true;
            break;
        }
    }
    if(!fontLoaded) {
        io.Fonts->AddFontDefault();
    }

    _imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(),
                                       Vector2{windowSize()}/dpiScaling(),
                                       windowSize(), framebufferSize());

    // Initialize baseline UI scale
    const Float baseHeight = 720.0f;
    ImGui::GetIO().FontGlobalScale = Math::max(0.75f, Float(windowSize().y()) / baseHeight);
    
    _menuManager.applyCustomTheme();

    _menuManager.setOnCreateWorld([this](const WorldCreationParams& params) {
        auto randGen = dynamic_cast<RandomWorldGenerator*>(_world.generator());
        if(randGen) {
            randGen->setArchetype(static_cast<WorldArchetype>(params.archetype));
            randGen->setFlatness(params.flatness);
            randGen->setMountainScale(params.mountainScale);
            randGen->setRiverIntensity(params.riverIntensity);
            randGen->setForestDensity(params.forestDensity);
            randGen->setResourceDensity(params.resourceDensity);
            randGen->setSeaLevel(params.seaLevel);
            randGen->setSeed(params.seed);
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

    // World Input callback: Left-click places road or demolishes when in build mode
    _worldInputHandler.setOnTileLeftClick([this](const World::RaycastHit& hit, Input::Pointer) {
        if(_buildSystem.isBuilding()) {
            _buildSystem.executeAction(_world, hit);
        }
    });

    Debug{} << "CityBuilder Initialized!";
    _timeline.start();
}

void MyApplication::viewportEvent(ViewportEvent& event) {
    _renderer.setViewport(event.framebufferSize());
    _camera.setViewport(event.framebufferSize());

    // Responsive UI scale based on window height (baseline 720p)
    const Float baseHeight = 720.0f;
    const Float uiScale = Math::max(0.75f, Float(event.windowSize().y()) / baseHeight);
    ImGui::GetIO().FontGlobalScale = uiScale;

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
                    event.windowSize(), event.framebufferSize());
}

void MyApplication::keyPressEvent(KeyEvent& event) {
    if(_imgui.handleKeyPressEvent(event)) return;

    if(event.key() == Sdl2Application::Key::Esc) {
        if(_buildSystem.isBuilding()) {
            _buildSystem.setActiveTool(BuildTool::Inspect);
            event.setAccepted();
            return;
        }
        _menuManager.toggleMenu();
        event.setAccepted();
        return;
    }

    // Number hotkeys for Build tools: [1] Road, [2] House, [3] Cottage, [4] City Center, [5] Plaza, [6] Demolish, [R] Rotate
    if(event.key() == Sdl2Application::Key::One) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::RoadCobblestone ? BuildTool::Inspect : BuildTool::RoadCobblestone
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Two) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::House2x2 ? BuildTool::Inspect : BuildTool::House2x2
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Three) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::HouseStandalone2x2 ? BuildTool::Inspect : BuildTool::HouseStandalone2x2
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Four) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::CityCenter2x3 ? BuildTool::Inspect : BuildTool::CityCenter2x3
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Five) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::CityPlaza4x2 ? BuildTool::Inspect : BuildTool::CityPlaza4x2
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Six) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::Warehouse4x4 ? BuildTool::Inspect : BuildTool::Warehouse4x4
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::Seven) {
        _buildSystem.setActiveTool(
            _buildSystem.activeTool() == BuildTool::Demolish ? BuildTool::Inspect : BuildTool::Demolish
        );
        event.setAccepted();
        return;
    }
    if(event.key() == Sdl2Application::Key::R && _buildSystem.isBuilding()) {
        _buildSystem.rotateBuilding();
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
    _elapsedTime += dt;

    _timeManager.update(dt);
    _renderer.setTime(_elapsedTime);
    _renderer.update(dt, _world);

    // Synchronize atmospheric lighting & sky clear color with TimeManager
    GL::Renderer::setClearColor(_timeManager.skyColor());
    _renderer.setLightColor(_timeManager.lightColor());
    _renderer.setAmbientColor(_timeManager.ambientColor());
    _renderer.terrainShader()
        .setLightDirection(_timeManager.lightDirection())
        .setSkyColor(_timeManager.skyColor());

    _imgui.newFrame();

    if(ImGui::GetIO().WantTextInput) {
        startTextInput();
    } else {
        stopTextInput();
    }

    if(_menuManager.isMenuOpen()) {
        _camera.rotate(Rad{0.08f * dt}, 0.0_radf);

        _renderer.render(_world, _camera);

        _menuManager.draw(windowSize());
    } else {
        const Vector3 minBounds{_world.minWorldX(), 0.0f, _world.minWorldZ()};
        const Vector3 maxBounds{_world.maxWorldX(), 0.0f, _world.maxWorldZ()};
        _cameraController.update(_input, dt, minBounds, maxBounds);

        _worldInputHandler.update(_input, framebufferSize());

        PlacementValidation validation = _buildSystem.validatePlacement(_world, _worldInputHandler.hoverHit());

        _renderer.render(_world, _camera,
                         _worldInputHandler.hoverHit(),
                         _worldInputHandler.selectedHit(),
                         _buildSystem.toolFootprint(),
                         !_buildSystem.isBuilding() || validation.valid);

        _gameHUD.update(dt);
        _gameHUD.draw(windowSize(), _renderer.lastDrawnChunks(), _world.chunks().size(),
                      &_timeManager, &_world, _worldInputHandler.selectedHit(),
                      &_buildSystem, validation);
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
