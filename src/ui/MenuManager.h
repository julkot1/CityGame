#pragma once

#include <functional>
#include <string>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "ui/UIStyle.h"

namespace Magnum {

enum class AppState {
    MainMenu,
    Playing
};

struct WorldCreationParams {
    int seed{1337};
    std::string seedString{"1337"};
    float seaLevel{4.5f};
    int worldWidthChunks{8};
    int worldHeightChunks{8};
};

/**
 * Manages game state and renders Main Menu and World Creation dialog.
 */
class MenuManager {
    public:
        explicit MenuManager();

        AppState state() const { return _state; }
        void setState(AppState state) { _state = state; }
        void applyCustomTheme();

        bool isMenuOpen() const { return _state == AppState::MainMenu; }
        void toggleMenu();

        // Callbacks
        void setOnCreateWorld(std::function<void(const WorldCreationParams&)> cb) { _onCreateWorld = std::move(cb); }
        void setOnContinue(std::function<void()> cb) { _onContinue = std::move(cb); }
        void setOnSaveWorld(std::function<void()> cb) { _onSaveWorld = std::move(cb); }
        void setOnExit(std::function<void()> cb) { _onExit = std::move(cb); }

        // Render UI
        void draw(const Vector2i& windowSize);

    private:
        void drawMainMenu(const Vector2i& windowSize);
        void drawWorldCreationDialog(const Vector2i& windowSize);

        AppState _state{AppState::MainMenu};
        bool _showWorldCreationDialog{false};

        WorldCreationParams _creationParams;

        std::function<void(const WorldCreationParams&)> _onCreateWorld;
        std::function<void()> _onContinue;
        std::function<void()> _onSaveWorld;
        std::function<void()> _onExit;
};

}
