#include "ui/GameHUD.h"
#include "ui/UIStyle.h"
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

void GameHUD::draw(const Vector2i& windowSize, UnsignedInt renderedChunks, UnsignedInt totalChunks, TimeManager* timeManager) {
    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    // ==========================================
    // 1. TOP-LEFT HUD: Performance & Map Stats
    // ==========================================
    ImVec2 overlayPos(padding, padding);
    ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    if(ImGui::Begin("##GameStatsHUD", nullptr, flags)) {
        // Pill Badge for Status
        ImGui::TextColored(UIStyle::Colors::AmberGold, "SYS // MONITOR");
        ImGui::Separator();

        // Color-code FPS: green (>60), yellow (>30), red (<=30)
        ImVec4 fpsColor;
        if(_currentFps >= 60.0f) {
            fpsColor = UIStyle::Colors::NeonGreen;
        } else if(_currentFps >= 30.0f) {
            fpsColor = UIStyle::Colors::AmberGold;
        } else {
            fpsColor = UIStyle::Colors::SoftRed;
        }

        ImGui::TextColored(fpsColor, "%.1f FPS", static_cast<double>(_currentFps));
        ImGui::SameLine();
        ImGui::TextDisabled("| %.2f ms", static_cast<double>(_currentFrameTimeMs));

        ImGui::Spacing();
        ImGui::Text("Chunks: %u / %u", renderedChunks, totalChunks);
        ImGui::SameLine();
        ImGui::TextDisabled("(Culled: %u)", totalChunks - renderedChunks);

        ImGui::Separator();
        ImGui::TextDisabled("[ESC] Menu");

        ImGui::End();
    }

    // ==========================================
    // 2. TOP-RIGHT HUD: Celestial & Time Controls
    // ==========================================
    if(timeManager) {
        ImVec2 topRightPos(windowSize.x() - padding, padding);
        ImGui::SetNextWindowPos(topRightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.85f);

        if(ImGui::Begin("##GameTimeHUD", nullptr, flags)) {
            std::string timeStr = timeManager->formattedTime();
            bool night = timeManager->isNight();
            ImVec4 timeBadgeColor = night ? UIStyle::Colors::SkyBlue : UIStyle::Colors::AmberGold;

            ImGui::TextColored(timeBadgeColor, "%s  %s", night ? "[MOON]" : "[SUN]", timeStr.c_str());
            ImGui::Separator();

            // Play / Pause button
            if(ImGui::Button(timeManager->isPaused() ? "  Play  " : " Pause ")) {
                timeManager->togglePaused();
            }
            ImGui::SameLine();
            if(ImGui::Button(" 1x ")) timeManager->setTimeSpeed(1.0f);
            ImGui::SameLine();
            if(ImGui::Button(" 5x ")) timeManager->setTimeSpeed(5.0f);
            ImGui::SameLine();
            if(ImGui::Button(" 20x ")) timeManager->setTimeSpeed(20.0f);

            // Slider to scrub time of day
            float t = timeManager->timeOfDay();
            ImGui::PushItemWidth(170.0f);
            if(ImGui::SliderFloat("##TimeOfDay", &t, 0.0f, 1.0f, "Scrub: %.2f")) {
                timeManager->setTimeOfDay(t);
            }
            ImGui::PopItemWidth();

            ImGui::End();
        }
    }
}

}
