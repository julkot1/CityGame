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
#include "simulation/SimulationThread.h"
#include "threading/SimulationCommand.h"
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
#include <shared_mutex>
#include <cmath>

using namespace Magnum;

class MyApplication: public Platform::Application {
    public:
        explicit MyApplication(const Arguments& arguments);
        ~MyApplication();

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
        TimeManager _renderTimeManager;
        BuildSystem _buildSystem;

        World _world{1.0f};
        SimulationThread _simThread{_world};

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

    // Load or generate initial world
    WorldStorage::loadOrGenerate(_world, "world.save");

    // Initialize renderer bounds and upload all initial chunk meshes to GPU
    _renderer.setWorldBounds(_world.minWorldX(), _world.maxWorldX(),
                             _world.minWorldZ(), _world.maxWorldZ(),
                             _world.tileSpacing());
    for(auto& meshData : _world.buildAllCpuMeshes()) {
        _renderer.uploadChunkMesh(std::move(meshData));
    }

    // Start simulation worker thread
    _simThread.start();

    _menuManager.setOnCreateWorld([this](const WorldCreationParams& params) {
        _renderer.clearRenderChunks();
        Float halfW = (params.worldWidthChunks * Chunk::Size * _world.tileSpacing()) * 0.5f;
        Float halfH = (params.worldHeightChunks * Chunk::Size * _world.tileSpacing()) * 0.5f;
        _renderer.setWorldBounds(-halfW, halfW, -halfH, halfH, _world.tileSpacing());

        SimulationCommand cmd;
        cmd.type = SimulationCommandType::CreateNewWorld;
        cmd.worldParams = params;
        _simThread.postCommand(cmd);

        _camera.setTarget(Vector3{0.0f});
    });
    _menuManager.setOnSaveWorld([this]() {
        SimulationCommand cmd;
        cmd.type = SimulationCommandType::SaveWorld;
        cmd.filename = "world.save";
        _simThread.postCommand(cmd);
    });
    _menuManager.setOnExit([this]() {
        _simThread.stop();
        exit();
    });

    // World Input callback: Left-click places road or demolishes when in build mode
    _worldInputHandler.setOnTileLeftClick([this](const World::RaycastHit& hit, Input::Pointer) {
        if(_buildSystem.isBuilding() && hit.hit) {
            SimulationCommand cmd;
            if(_buildSystem.activeTool() == BuildTool::Demolish) {
                cmd.type = SimulationCommandType::Demolish;
            } else {
                cmd.type = SimulationCommandType::PlaceStructure;
                cmd.tool = _buildSystem.activeTool();
                cmd.orientation = _buildSystem.buildingOrientation();
            }
            cmd.worldX = hit.worldTileX;
            cmd.worldZ = hit.worldTileZ;
            _simThread.postCommand(cmd);
        }
    });

    _worldInputHandler.setOnTileRightClick([this](const World::RaycastHit&, Input::Pointer) {
        if(_buildSystem.isBuilding()) {
            _buildSystem.setActiveTool(BuildTool::Inspect);
        }
    });

    Debug{} << "CityBuilder Initialized with Multithreaded Simulation & Renderer!";
    _timeline.start();
}

MyApplication::~MyApplication() {
    _simThread.stop();
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

    // 1. Drain pending finished CPU chunk meshes from simulation and upload to GPU
    std::vector<ChunkMeshData> pendingMeshes;
    _simThread.drainMeshUploads(pendingMeshes);
    for(auto& meshData : pendingMeshes) {
        _renderer.uploadChunkMesh(std::move(meshData));
    }

    // 2. Fetch thread-safe lighting & celestial snapshot and chimney positions
    TimeLightingSnapshot snap = _simThread.getLightingSnapshot();
    std::vector<Vector3> chimneys = _simThread.getChimneyPositions();

    _renderer.setTime(_elapsedTime);
    _renderer.update(dt, chimneys);

    // Synchronize atmospheric lighting & sky clear color with snapshot
    GL::Renderer::setClearColor(snap.skyColor);
    _renderer.setLightColor(snap.lightColor);
    _renderer.setAmbientColor(snap.ambientColor);
    _renderer.terrainShader()
        .setLightDirection(snap.lightDirection)
        .setSkyColor(snap.skyColor);

    // Sync render-side time manager from snapshot for HUD display
    _renderTimeManager.setTimeOfDay(snap.timeOfDay);
    if(snap.isPaused != _renderTimeManager.isPaused()) {
        _renderTimeManager.togglePaused();
    }
    _renderTimeManager.setTimeSpeed(snap.timeSpeed);

    _imgui.newFrame();

    if(ImGui::GetIO().WantTextInput) {
        startTextInput();
    } else {
        stopTextInput();
    }

    if(_menuManager.isMenuOpen()) {
        _camera.rotate(Rad{0.08f * dt}, 0.0_radf);

        _renderer.render(_camera);

        _menuManager.draw(windowSize());
    } else {
        Float minX, maxX, minZ, maxZ;
        {
            std::shared_lock<std::shared_mutex> lock(_simThread.worldMutex());
            minX = _world.minWorldX();
            maxX = _world.maxWorldX();
            minZ = _world.minWorldZ();
            maxZ = _world.maxWorldZ();
        }
        const Vector3 minBounds{minX, 0.0f, minZ};
        const Vector3 maxBounds{maxX, 0.0f, maxZ};
        _cameraController.update(_input, dt, minBounds, maxBounds);

        PlacementValidation validation;
        {
            std::shared_lock<std::shared_mutex> lock(_simThread.worldMutex());
            _worldInputHandler.update(_input, framebufferSize());
            validation = _buildSystem.validatePlacement(_world, _worldInputHandler.hoverHit());
        }

        _renderer.render(_camera,
                         _worldInputHandler.hoverHit(),
                         _worldInputHandler.selectedHit(),
                         _buildSystem.toolFootprint(),
                         !_buildSystem.isBuilding() || validation.valid);

        _gameHUD.update(dt);

        bool hudPausedBefore = _renderTimeManager.isPaused();
        Float hudSpeedBefore = _renderTimeManager.timeSpeed();
        Float hudTimeBefore = _renderTimeManager.timeOfDay();

        {
            std::shared_lock<std::shared_mutex> lock(_simThread.worldMutex());
            _gameHUD.draw(windowSize(), _renderer.lastDrawnChunks(),
                          _renderer.totalRenderChunks(),
                          &_renderTimeManager, &_world, _worldInputHandler.selectedHit(),
                          &_buildSystem, validation,
                          _simThread.tps(), _simThread.tickDurationMs());
        }

        if(_renderTimeManager.isPaused() != hudPausedBefore) {
            SimulationCommand cmd;
            cmd.type = SimulationCommandType::TimeTogglePause;
            _simThread.postCommand(cmd);
        }
        if(_renderTimeManager.timeSpeed() != hudSpeedBefore) {
            SimulationCommand cmd;
            cmd.type = SimulationCommandType::TimeSetSpeed;
            cmd.timeValue = _renderTimeManager.timeSpeed();
            _simThread.postCommand(cmd);
        }
        if(std::abs(_renderTimeManager.timeOfDay() - hudTimeBefore) > 0.001f) {
            SimulationCommand cmd;
            cmd.type = SimulationCommandType::TimeSetTimeOfDay;
            cmd.timeValue = _renderTimeManager.timeOfDay();
            _simThread.postCommand(cmd);
        }
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
        setWindowTitle(Utility::formatString("City Builder - {:.1f} FPS ({:.2f} ms) | Sim: {:.0f} TPS ({:.2f} ms)",
                                             fps, frameTimeMs, _simThread.tps(), _simThread.tickDurationMs()));

        _fpsFrameCount = 0;
        _fpsTimeAccumulator = 0.0f;
    }

    swapBuffers();
    _timeline.nextFrame();
    _input.nextFrame();
    redraw();
}

MAGNUM_APPLICATION_MAIN(MyApplication)
