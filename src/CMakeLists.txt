find_package(Magnum REQUIRED GL Sdl2Application)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(app
    main.cpp
    camera/Camera.h
    camera/Camera.cpp
    camera/CameraController.h
    camera/CameraController.cpp
    input/Input.h
    input/Input.cpp
    rendering/Renderer.h
    rendering/Renderer.cpp
    rendering/TerrainShader.h
    rendering/TerrainShader.cpp
    world/TimeManager.h
    world/TimeManager.cpp
    world/Tile.h
    world/Chunk.h
    world/Chunk.cpp
    world/World.h
    world/World.cpp
    world/SimplexNoise.h
    world/SimplexNoise.cpp
    world/WorldGenerator.h
    world/RandomWorldGenerator.h
    world/RandomWorldGenerator.cpp
    world/WorldStorage.h
    world/WorldStorage.cpp
    world/WorldInputHandler.h
    world/WorldInputHandler.cpp
    ui/MenuManager.h
    ui/MenuManager.cpp
    ui/GameHUD.h
    ui/GameHUD.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(app PRIVATE
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::Shaders
    MagnumIntegration::ImGui)

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app)
