#include "ui/MenuManager.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <random>

namespace Magnum {

MenuManager::MenuManager() = default;

void MenuManager::applyCustomTheme() {
    UIStyle::apply();
}
void MenuManager::toggleMenu() {
    _state = (_state == AppState::MainMenu) ? AppState::Playing : AppState::MainMenu;
    if(_state == AppState::Playing) {
        _showWorldCreationDialog = false;
    }
}

void MenuManager::draw(const Vector2i& windowSize) {
    if(_state != AppState::MainMenu) return;

    if(_showWorldCreationDialog) {
        drawWorldCreationDialog(windowSize);
    } else {
        drawMainMenu(windowSize);
    }
}

void MenuManager::drawMainMenu(const Vector2i& windowSize) {
    Vector2 winSize{windowSize};
    ImVec2 menuSize(360.0f, 360.0f);
    ImVec2 menuPos((winSize.x() - menuSize.x) * 0.5f, (winSize.y() - menuSize.y) * 0.5f);

    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar; // Borderless card

    if(ImGui::Begin("##MainMenuPanel", nullptr, flags)) {
        ImGui::Dummy(ImVec2(0, 10));

        // Stylized Title Header
        float windowWidth = ImGui::GetWindowSize().x;
        const char* title = "C I T Y   B U I L D E R";
        float textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(UIStyle::Colors::AmberGold, "%s", title);

        const char* subtitle = "--- SIMULATION & STRATEGY ---";
        float subWidth = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX((windowWidth - subWidth) * 0.5f);
        ImGui::TextDisabled("%s", subtitle);

        ImGui::Dummy(ImVec2(0, 12));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 8));

        ImVec2 buttonSize(windowWidth - 36.0f, 42.0f);

        ImGui::SetCursorPosX(18.0f);
        if(ImGui::Button("NEW WORLD", buttonSize)) {
            _showWorldCreationDialog = true;
        }

        ImGui::SetCursorPosX(18.0f);
        if(ImGui::Button("CONTINUE", buttonSize)) {
            if(_onContinue) _onContinue();
            _state = AppState::Playing;
        }

        ImGui::SetCursorPosX(18.0f);
        if(ImGui::Button("SAVE WORLD", buttonSize)) {
            if(_onSaveWorld) _onSaveWorld();
        }

        ImGui::SetCursorPosX(18.0f);
        if(ImGui::Button("EXIT TO DESKTOP", buttonSize)) {
            if(_onExit) _onExit();
        }

        ImGui::End();
    }
}

void MenuManager::drawWorldCreationDialog(const Vector2i& windowSize) {
    Vector2 winSize{windowSize};
    ImVec2 dialogSize(440.0f, 380.0f);
    ImVec2 dialogPos((winSize.x() - dialogSize.x) * 0.5f, (winSize.y() - dialogSize.y) * 0.5f);

    ImGui::SetNextWindowPos(dialogPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dialogSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar;

    if(ImGui::Begin("##CreateWorldDialog", nullptr, flags)) {
        ImGui::Dummy(ImVec2(0, 5));

        // Header
        ImGui::TextColored(UIStyle::Colors::AmberGold, "NEW WORLD CREATION");
        ImGui::TextDisabled("Configure procedural terrain parameters");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 6));

        // Seed parameter
        ImGui::Text("WORLD SEED");
        ImGui::PushItemWidth(260.0f);
        ImGui::InputText("##seed", &_creationParams.seedString);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if(ImGui::Button("Randomize", ImVec2(120.0f, 0.0f))) {
            std::random_device rd;
            int seed = static_cast<int>(rd() % 100000);
            _creationParams.seed = seed;
            _creationParams.seedString = std::to_string(seed);
        }

        ImGui::Dummy(ImVec2(0, 6));

        ImGui::Text("SEA LEVEL / WATER COVERAGE");
        ImGui::SliderFloat("##sealevel", &_creationParams.seaLevel, 2.0f, 8.0f, "%.1f m");

        ImGui::Dummy(ImVec2(0, 6));

        // Map Size
        ImGui::Text("WORLD DIMENSIONS");
        const char* sizes[] = {"Small (4x4 chunks)", "Standard (8x8 chunks)", "Large (12x12 chunks)"};
        static int currentSizeIndex = 1;
        if(ImGui::Combo("##worldsize", &currentSizeIndex, sizes, IM_ARRAYSIZE(sizes))) {
            if(currentSizeIndex == 0) {
                _creationParams.worldWidthChunks = 4;
                _creationParams.worldHeightChunks = 4;
            } else if(currentSizeIndex == 1) {
                _creationParams.worldWidthChunks = 8;
                _creationParams.worldHeightChunks = 8;
            } else {
                _creationParams.worldWidthChunks = 12;
                _creationParams.worldHeightChunks = 12;
            }
        }

        ImGui::Dummy(ImVec2(0, 16));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 6));

        // Action buttons: Cancel and Generate
        float btnWidth = 190.0f;
        ImGui::SetCursorPosX((dialogSize.x - (btnWidth * 2.0f + 12.0f)) * 0.5f);

        if(ImGui::Button("CANCEL", ImVec2(btnWidth, 40.0f))) {
            _showWorldCreationDialog = false;
        }
        ImGui::SameLine();
        if(ImGui::Button("CREATE WORLD", ImVec2(btnWidth, 40.0f))) {
            try {
                _creationParams.seed = std::stoi(_creationParams.seedString);
            } catch(...) {
                _creationParams.seed = 1337;
            }
            if(_onCreateWorld) {
                _onCreateWorld(_creationParams);
            }
            _showWorldCreationDialog = false;
            _state = AppState::Playing;
        }

        ImGui::End();
    }
}

}
