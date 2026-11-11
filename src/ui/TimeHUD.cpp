#include "ui/TimeHUD.h"
#include "ui/UIStyle.h"
#include <imgui.h>

namespace Magnum {

TimeHUD::TimeHUD() = default;

void TimeHUD::draw(const Vector2i& windowSize, TimeManager& timeManager) {
    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImVec2 topRightPos(windowSize.x() - padding, padding);
    ImGui::SetNextWindowPos(topRightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.85f);

    if(ImGui::Begin("##TimeHUD", nullptr, flags)) {
        std::string timeStr = timeManager.formattedTime();
        bool night = timeManager.isNight();
        ImVec4 timeBadgeColor = night ? UIStyle::Colors::SkyBlue : UIStyle::Colors::AmberGold;

        ImGui::TextColored(timeBadgeColor, "%s  %s", night ? "[MOON]" : "[SUN]", timeStr.c_str());
        ImGui::Separator();

        // Play / Pause button
        if(ImGui::Button(timeManager.isPaused() ? "  Play  " : " Pause ")) {
            timeManager.togglePaused();
        }
        ImGui::SameLine();
        if(ImGui::Button(" 1x ")) timeManager.setTimeSpeed(1.0f);
        ImGui::SameLine();
        if(ImGui::Button(" 5x ")) timeManager.setTimeSpeed(5.0f);
        ImGui::SameLine();
        if(ImGui::Button(" 20x ")) timeManager.setTimeSpeed(20.0f);

        // Slider to scrub time of day
        float t = timeManager.timeOfDay();
        ImGui::PushItemWidth(170.0f);
        if(ImGui::SliderFloat("##TimeOfDay", &t, 0.0f, 1.0f, "Scrub: %.2f")) {
            timeManager.setTimeOfDay(t);
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }
}

}
