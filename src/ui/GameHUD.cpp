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

void GameHUD::draw(const Vector2i& windowSize, UnsignedInt renderedChunks, UnsignedInt totalChunks, TimeManager* timeManager) {
    // Top-left corner overlay
    constexpr float padding = 12.0f;
    ImVec2 overlayPos(padding, padding);

    ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.70f); // Semi-transparent dark background

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
    ImVec2 topRightPos(windowSize.x() - padding, padding);
    ImGui::SetNextWindowPos(topRightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.70f); 
    if(ImGui::Begin("##GameTimeHUD", nullptr, flags)) {
         if(timeManager) {
            std::string timeStr = timeManager->formattedTime();
            const char* timeIcon = timeManager->isNight() ? "[Night]" : "[Day]";
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.4f, 1.0f), "Time: %s %s", timeStr.c_str(), timeIcon);

            // Play / Pause button
            if(ImGui::Button(timeManager->isPaused() ? "Play" : "Pause")) {
                timeManager->togglePaused();
            }
            ImGui::SameLine();
            if(ImGui::Button("1x")) timeManager->setTimeSpeed(1.0f);
            ImGui::SameLine();
            if(ImGui::Button("5x")) timeManager->setTimeSpeed(5.0f);
            ImGui::SameLine();
            if(ImGui::Button("20x")) timeManager->setTimeSpeed(20.0f);

            // Slider to scrub time of day
            float t = timeManager->timeOfDay();
            if(ImGui::SliderFloat("##TimeOfDay", &t, 0.0f, 1.0f, "Scrub: %.2f")) {
                timeManager->setTimeOfDay(t);
            }

        }
        ImGui::End();
    }
}

}
