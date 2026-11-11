#include "ui/BuildHUD.h"
#include "ui/UIStyle.h"
#include <imgui.h>

namespace Magnum {

BuildHUD::BuildHUD() = default;

void BuildHUD::draw(const Vector2i& windowSize) {
    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImVec2 bottomRightPos(windowSize.x() - padding, windowSize.y() - padding);
    ImGui::SetNextWindowPos(bottomRightPos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);

    if(ImGui::Begin("##BuildHUD", nullptr, flags)) {
        ImGui::TextColored(UIStyle::Colors::AmberGold, "BUILD MODE: ");
        ImGui::SameLine(0.0f, 0.0f); 
        ImGui::TextColored(UIStyle::Colors::SkyBlue, "%s", selected.c_str());
        ImGui::Separator();
        if(ImGui::Button("Road"))
        {
            selected = "Road";
        }
        ImGui::SameLine();
        if(ImGui::Button("House"))
        {
            selected = "House";
        }
        ImGui::SameLine();
        if(ImGui::Button("None"))
        {
            selected = "None";
        }
        ImGui::End();
    }
}

void BuildHUD::update(Float dt) {
}


}
