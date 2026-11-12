#include "ui/PerformanceHUD.h"
#include "ui/UIStyle.h"
#include <imgui.h>

namespace Magnum {

PerformanceHUD::PerformanceHUD() = default;

void PerformanceHUD::update(Float dt) {
    _frameCount++;
    _timeAccumulator += dt;
    if(_timeAccumulator >= 0.5f) {
        _currentFps = Float(_frameCount) / _timeAccumulator;
        _currentFrameTimeMs = (_timeAccumulator / Float(_frameCount)) * 1000.0f;
        _frameCount = 0;
        _timeAccumulator = 0.0f;
    }
}

void PerformanceHUD::draw(const Vector2i&, UnsignedInt renderedChunks, UnsignedInt totalChunks,
                          Float simTPS, Float simTickDurationMs) {
    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImVec2 overlayPos(padding, padding);
    ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    if(ImGui::Begin("##PerformanceHUD", nullptr, flags)) {
        ImGui::TextColored(UIStyle::Colors::AmberGold, "SYS // MONITOR");
        ImGui::Separator();

        // Color-code FPS
        ImVec4 fpsColor;
        if(_currentFps >= 60.0f) {
            fpsColor = UIStyle::Colors::NeonGreen;
        } else if(_currentFps >= 30.0f) {
            fpsColor = UIStyle::Colors::AmberGold;
        } else {
            fpsColor = UIStyle::Colors::SoftRed;
        }

        ImGui::TextColored(fpsColor, "RENDER: %.1f FPS", static_cast<double>(_currentFps));
        ImGui::SameLine();
        ImGui::TextDisabled("| %.2f ms", static_cast<double>(_currentFrameTimeMs));

        // Display Simulation Thread TPS
        ImVec4 tpsColor = (simTPS >= 55.0f) ? UIStyle::Colors::NeonGreen : ((simTPS >= 30.0f) ? UIStyle::Colors::AmberGold : UIStyle::Colors::SoftRed);
        ImGui::TextColored(tpsColor, "SIM:    %.1f TPS", static_cast<double>(simTPS));
        ImGui::SameLine();
        ImGui::TextDisabled("| %.2f ms", static_cast<double>(simTickDurationMs));

        ImGui::Spacing();
        ImGui::Text("Chunks: %u / %u", renderedChunks, totalChunks);
        ImGui::SameLine();
        ImGui::TextDisabled("(Culled: %u)", totalChunks - renderedChunks);

        ImGui::Separator();
        ImGui::TextDisabled("[ESC] Menu");

        ImGui::End();
    }
}

}
