#include "ui/MenuManager.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <random>

namespace Magnum {

MenuManager::MenuManager() = default;

void MenuManager::applyCustomTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window & Backgrounds
    colors[ImGuiCol_WindowBg]             = ImVec4(0.11f, 0.12f, 0.14f, 0.94f); // Dark translucent
    colors[ImGuiCol_ChildBg]              = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.13f, 0.15f, 0.96f);
    colors[ImGuiCol_Border]               = ImVec4(0.24f, 0.27f, 0.32f, 0.80f); // Subtle border
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text
    colors[ImGuiCol_Text]                 = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.52f, 0.56f, 1.00f);

    // Headers & Title Bars
    colors[ImGuiCol_TitleBg]              = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.11f, 0.13f, 0.75f);

    // Buttons
    colors[ImGuiCol_Button]               = ImVec4(0.22f, 0.25f, 0.30f, 1.00f); // Normal button
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.32f, 0.37f, 0.45f, 1.00f); // Hovered
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.40f, 0.46f, 0.56f, 1.00f); // Pressed

    // Frame backgrounds (Text inputs, checkboxes, combos)
    colors[ImGuiCol_FrameBg]              = ImVec4(0.16f, 0.18f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.26f, 0.30f, 0.36f, 1.00f);

    // Sliders & Tabs (Accent colors: Golden Amber)
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(1.00f, 0.80f, 0.30f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.11f, 0.12f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.32f, 0.36f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.40f, 0.45f, 0.52f, 1.00f);

    style.WindowRounding    = 12.0f; // Outer main menu / popup window corners
    style.ChildRounding     = 8.0f;  // Internal child panels / sub-windows
    style.PopupRounding     = 10.0f; // Combobox dropdowns, tooltips, context menus

    // --- 2. Interactive Controls (Buttons, Inputs, Sliders) ---
    style.FrameRounding     = 8.0f;  // Buttons, text inputs, checkboxes, combos
    style.GrabRounding      = 8.0f;  // The draggable knob on sliders and scrollbars
    style.ScrollbarRounding = 12.0f; // Scrollbar track & thumb (pill shape)
    style.TabRounding       = 8.0f;  // Tab bars
    
    // --- 3. Borders & Shadows ---
    style.WindowBorderSize  = 1.5f;  // 0.0f = borderless modern flat look, >0.0f = outlined
    style.FrameBorderSize   = 1.0f;  // Subtle border around input boxes & buttons
    style.PopupBorderSize   = 1.5f;
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
    ImVec2 menuSize(320.0f, 320.0f);
    ImVec2 menuPos((winSize.x() - menuSize.x) * 0.5f, (winSize.y() - menuSize.y) * 0.5f);

    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse;

    if(ImGui::Begin("CITY BUILDER", nullptr, flags)) {
        ImGui::Dummy(ImVec2(0, 10));

        // Center Title Text
        float windowWidth = ImGui::GetWindowSize().x;
        const char* title = "=== MAIN MENU ===";
        float textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", title);

        ImGui::Dummy(ImVec2(0, 15));

        ImVec2 buttonSize(windowWidth - 30.0f, 42.0f);

        ImGui::SetCursorPosX(15.0f);
        if(ImGui::Button("NEW WORLD...", buttonSize)) {
            _showWorldCreationDialog = true;
        }

        ImGui::SetCursorPosX(15.0f);
        if(ImGui::Button("CONTINUE", buttonSize)) {
            if(_onContinue) _onContinue();
            _state = AppState::Playing;
        }

        ImGui::SetCursorPosX(15.0f);
        if(ImGui::Button("SAVE WORLD", buttonSize)) {
            if(_onSaveWorld) _onSaveWorld();
        }

        ImGui::SetCursorPosX(15.0f);
        if(ImGui::Button("EXIT TO DESKTOP", buttonSize)) {
            if(_onExit) _onExit();
        }

        ImGui::End();
    }
}

void MenuManager::drawWorldCreationDialog(const Vector2i& windowSize) {
    Vector2 winSize{windowSize};
    ImVec2 dialogSize(420.0f, 340.0f);
    ImVec2 dialogPos((winSize.x() - dialogSize.x) * 0.5f, (winSize.y() - dialogSize.y) * 0.5f);

    ImGui::SetNextWindowPos(dialogPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dialogSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse;

    if(ImGui::Begin("CREATE NEW WORLD", nullptr, flags)) {
        ImGui::Dummy(ImVec2(0, 5));

        // Seed parameter
        ImGui::Text("World Seed:");
        ImGui::InputText("##seed", &_creationParams.seedString);
        ImGui::SameLine();
        if(ImGui::Button("Random")) {
            std::random_device rd;
            int seed = static_cast<int>(rd() % 100000);
            _creationParams.seed = seed;
            _creationParams.seedString = std::to_string(seed);
        }

        ImGui::Dummy(ImVec2(0, 8));

        ImGui::Text("Sea Level (Water Coverage):");
        ImGui::SliderFloat("##sealevel", &_creationParams.seaLevel, 2.0f, 8.0f, "%.1f");

        ImGui::Dummy(ImVec2(0, 8));

        // Map Size
        ImGui::Text("World Size:");
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

        ImGui::Dummy(ImVec2(0, 20));

        // Action buttons: Cancel and Generate
        float btnWidth = 180.0f;
        ImGui::SetCursorPosX((dialogSize.x - (btnWidth * 2.0f + 15.0f)) * 0.5f);

        if(ImGui::Button("Cancel", ImVec2(btnWidth, 38.0f))) {
            _showWorldCreationDialog = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Generate World", ImVec2(btnWidth, 38.0f))) {
            if(_onCreateWorld) {
                _creationParams.seed = std::stoi(_creationParams.seedString);
                _onCreateWorld(_creationParams);
            }
            _showWorldCreationDialog = false;
            _state = AppState::Playing;
        }

        ImGui::End();
    }
}

}
