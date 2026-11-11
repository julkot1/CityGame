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
    ImVec2 dialogSize(480.0f, 580.0f);
    ImVec2 dialogPos((winSize.x() - dialogSize.x) * 0.5f, (winSize.y() - dialogSize.y) * 0.5f);

    ImGui::SetNextWindowPos(dialogPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dialogSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar;

    if(ImGui::Begin("##CreateWorldDialog", nullptr, flags)) {
        ImGui::Dummy(ImVec2(0, 4));

        // Header
        ImGui::TextColored(UIStyle::Colors::AmberGold, "NEW WORLD CHARTER // TERRAIN SURVEY");
        ImGui::TextDisabled("Configure 19th-century geological & resource parameters");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));

        // 1. Seed parameter
        ImGui::Text("WORLD SEED");
        ImGui::PushItemWidth(300.0f);
        ImGui::InputText("##seed", &_creationParams.seedString);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if(ImGui::Button("Randomize", ImVec2(130.0f, 0.0f))) {
            std::random_device rd;
            int seed = static_cast<int>(rd() % 100000);
            _creationParams.seed = seed;
            _creationParams.seedString = std::to_string(seed);
        }

        ImGui::Dummy(ImVec2(0, 4));

        // 2. Geography Archetype
        ImGui::Text("GEOGRAPHY ARCHETYPE");
        const char* archetypes[] = {
            "River Basin & Alluvial Valleys (Balanced)",
            "Coastal Archipelago & Harbors (Islands)",
            "Highland Plateaus & Moorlands (Terraced)",
            "Great Industrial Plains (Expansive Flatlands)"
        };
        ImGui::PushItemWidth(-1.0f);
        ImGui::Combo("##archetype", &_creationParams.archetype, archetypes, IM_ARRAYSIZE(archetypes));
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 4));

        // 3. Map Size
        ImGui::Text("WORLD DIMENSIONS");
        const char* sizes[] = {
            "Small (4x4 chunks)",
            "Standard (8x8 chunks)",
            "Large (12x12 chunks)",
            "Grand Empire (16x16 chunks)"
        };
        static int currentSizeIndex = 1;
        ImGui::PushItemWidth(-1.0f);
        if(ImGui::Combo("##worldsize", &currentSizeIndex, sizes, IM_ARRAYSIZE(sizes))) {
            const int dimTable[] = {4, 8, 12, 16};
            _creationParams.worldWidthChunks = dimTable[currentSizeIndex];
            _creationParams.worldHeightChunks = dimTable[currentSizeIndex];
        }
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 6));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));

        // 4. Geological & Ecological Sliders
        ImGui::PushItemWidth(-1.0f);

        ImGui::Text("PLATEAU FLATNESS (CITY BUILDABILITY)");
        ImGui::SliderFloat("##flatness", &_creationParams.flatness, 0.0f, 1.0f, "%.2f");

        ImGui::Text("SEA LEVEL / WATER COVERAGE");
        ImGui::SliderFloat("##sealevel", &_creationParams.seaLevel, 2.5f, 7.5f, "%.1f m");

        ImGui::Text("MOUNTAIN & RIDGE RELIEF");
        ImGui::SliderFloat("##mountains", &_creationParams.mountainScale, 0.0f, 2.0f, "%.2fx");

        ImGui::Text("RIVER & ESTUARY NETWORK");
        ImGui::SliderFloat("##rivers", &_creationParams.riverIntensity, 0.0f, 2.0f, "%.2fx");

        ImGui::Text("WOODLAND & FLORA DENSITY");
        ImGui::SliderFloat("##forests", &_creationParams.forestDensity, 0.0f, 2.0f, "%.2fx");

        ImGui::Text("MINERAL SEAMS (COAL / IRON / COPPER)");
        ImGui::SliderFloat("##resources", &_creationParams.resourceDensity, 0.0f, 2.0f, "%.2fx");

        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 6));

        // Action buttons: Cancel and Generate
        float btnWidth = 210.0f;
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
