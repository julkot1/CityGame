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
    world/Tile.h
    world/Chunk.h
    world/Chunk.cpp
    world/World.h
    world/World.cpp
    world/WorldInputHandler.h
    world/WorldInputHandler.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(app PRIVATE
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::Shaders)

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app)
