#include "ui/GameHUD.h"
#include <imgui.h>

namespace Magnum {

GameHUD::GameHUD() = default;

void GameHUD::update(Float dt) {
    _frameCount++;
    _timeAccumulator += dt;
    if(_timeAccumulator >= 0.5f) {
        _currentFps = Float(_frameCount) / _timeAccumulator;
        _currentFrameTimeMs = (_timeAccumulator / Float(_frameCount)) * 1000.0f;
        _frameCount = 0;
        _timeAccumulator = 0.0f;
    }
}

void GameHUD::draw(const Vector2i& windowSize, UnsignedInt renderedChunks, UnsignedInt totalChunks) {
    // Top-left corner overlay
    constexpr float padding = 12.0f;
    ImVec2 overlayPos(padding, padding);

    ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.65f); // Semi-transparent dark background

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    if(ImGui::Begin("##GameStatsHUD", nullptr, flags)) {
        // Color-code FPS: green (>60), yellow (>30), red (<=30)
        ImVec4 fpsColor;
        if(_currentFps >= 60.0f) {
            fpsColor = ImVec4(0.35f, 0.90f, 0.40f, 1.0f); // Bright green
        } else if(_currentFps >= 30.0f) {
            fpsColor = ImVec4(1.0f, 0.85f, 0.25f, 1.0f);  // Yellow/gold
        } else {
            fpsColor = ImVec4(0.95f, 0.35f, 0.35f, 1.0f); // Red
        }

        ImGui::TextColored(fpsColor, "FPS: %.1f", static_cast<double>(_currentFps));
        ImGui::SameLine();
        ImGui::TextDisabled("(%.2f ms)", static_cast<double>(_currentFrameTimeMs));

        ImGui::Separator();

        ImGui::Text("Chunks: %u / %u (Culled: %u)",
                    renderedChunks, totalChunks, totalChunks - renderedChunks);
        ImGui::TextDisabled("Press [ESC] for Menu");

        ImGui::End();
    }
}

}
