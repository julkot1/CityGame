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
    world/Entity.h
    world/EntityModelRegistry.h
    world/EntityModelRegistry.cpp
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
    world/BuildSystem.h
    world/BuildSystem.cpp
    ui/MenuManager.h
    ui/MenuManager.cpp
    ui/GameHUD.h
    ui/GameHUD.cpp
    ui/PerformanceHUD.h
    ui/PerformanceHUD.cpp
    ui/TimeHUD.h
    ui/TimeHUD.cpp
    ui/TileInspectorHUD.h
    ui/TileInspectorHUD.cpp
    ui/UIStyle.h
    ui/UIStyle.cpp
    ui/BuildHUD.h
    ui/BuildHUD.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(app PRIVATE
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::Shaders
    MagnumIntegration::ImGui)

# Automatically copy assets directory to build output so app finds them regardless of working dir
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:app>/assets)

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app)
