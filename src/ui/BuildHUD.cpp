#include "ui/BuildHUD.h"
#include "ui/UIStyle.h"
#include <imgui.h>

namespace Magnum {

BuildHUD::BuildHUD() = default;

void BuildHUD::update(Float) {
}

void BuildHUD::draw(const Vector2i& windowSize, BuildSystem& buildSystem, const PlacementValidation& validation) {
    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImVec2 bottomRightPos(windowSize.x() - padding, windowSize.y() - padding);
    ImGui::SetNextWindowPos(bottomRightPos, ImGuiCond_Always, ImVec2(1.0f, 1.0f)); // Pivot at bottom-right
    ImGui::SetNextWindowBgAlpha(0.88f);

    if(ImGui::Begin("##BuildHUD", nullptr, flags)) {
        ImGui::TextColored(UIStyle::Colors::AmberGold, "BUILD MODE: ");
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextColored(UIStyle::Colors::SkyBlue, "%s", buildSystem.toolName(buildSystem.activeTool()));

        ImGui::Separator();

        bool isRoad = (buildSystem.activeTool() == BuildTool::RoadCobblestone);
        bool isDemolish = (buildSystem.activeTool() == BuildTool::Demolish);
        bool isInspect = (buildSystem.activeTool() == BuildTool::Inspect);

        if(isRoad) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::AmberGold);
        if(ImGui::Button("[1] Road")) {
            buildSystem.setActiveTool(isRoad ? BuildTool::Inspect : BuildTool::RoadCobblestone);
        }
        if(isRoad) ImGui::PopStyleColor();

        ImGui::SameLine();

        if(isDemolish) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::SoftRed);
        if(ImGui::Button("[2] Demolish")) {
            buildSystem.setActiveTool(isDemolish ? BuildTool::Inspect : BuildTool::Demolish);
        }
        if(isDemolish) ImGui::PopStyleColor();

        ImGui::SameLine();

        if(isInspect) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::SkyBlue);
        if(ImGui::Button("Inspect")) {
            buildSystem.setActiveTool(BuildTool::Inspect);
        }
        if(isInspect) ImGui::PopStyleColor();

        if(buildSystem.isBuilding()) {
            ImGui::Separator();
            if(validation.valid) {
                ImGui::TextColored(UIStyle::Colors::NeonGreen, "%s", validation.reason.c_str());
            } else {
                ImGui::TextColored(UIStyle::Colors::SoftRed, "%s", validation.reason.c_str());
            }
        }

        ImGui::End();
    }
}

}
