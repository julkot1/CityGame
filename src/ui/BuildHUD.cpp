#include "ui/BuildHUD.h"
#include "ui/UIStyle.h"
#include "building/BuildingCatalog.h"
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

        auto categoryButton = [&](const char* label, BuildTypeHUD type) {
            bool isActive = (_activeBuiltType == type);
            if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::AmberGold);
            if(ImGui::Button(label)) {
                _activeBuiltType = isActive ? BuildTypeHUD::None : type;
            }
            if(isActive) ImGui::PopStyleColor();
        };

        categoryButton("[1] Infrastructure", BuildTypeHUD::Infrastructure);
        ImGui::SameLine();
        categoryButton("[2] Housing", BuildTypeHUD::Housing);
        ImGui::SameLine();
        categoryButton("[3] Commercial", BuildTypeHUD::Commercial);
        ImGui::SameLine();
        categoryButton("[4] Industrial", BuildTypeHUD::Industrial);
        ImGui::SameLine();
        categoryButton("[5] Civic", BuildTypeHUD::Civic);
        ImGui::SameLine();
        categoryButton("[6] Cultural", BuildTypeHUD::Cultural);
        ImGui::SameLine();

        // Demolish tool button
        bool isDemolish = (buildSystem.activeTool() == BuildTool::Demolish);
        if(isDemolish) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::SoftRed);
        if(ImGui::Button("[7] Demolish")) {
            buildSystem.setActiveTool(isDemolish ? BuildTool::Inspect : BuildTool::Demolish);
        }
        if(isDemolish) ImGui::PopStyleColor();

        ImGui::SameLine();

        // Inspect tool button
        bool isInspect = (buildSystem.activeTool() == BuildTool::Inspect);
        if(isInspect) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::SkyBlue);
        if(ImGui::Button("Inspect")) {
            buildSystem.setActiveTool(BuildTool::Inspect);
        }
        if(isInspect) ImGui::PopStyleColor();

        // Show Rotate button if the active blueprint is rotatable
        if(const BuildingBlueprint* activeBp = buildSystem.activeBlueprint()) {
            if(activeBp->rotatable) {
                ImGui::SameLine();
                if(ImGui::Button("[R] Rotate")) {
                    buildSystem.rotateBuilding();
                }
            }
        }

        // Submenu: render buildings belonging to the active category
        if(_activeBuiltType != BuildTypeHUD::None) {
            ImGui::Separator();

            BuiltType targetType = BuiltType::Infrastructure;
            switch(_activeBuiltType) {
                case BuildTypeHUD::Infrastructure: targetType = BuiltType::Infrastructure; break;
                case BuildTypeHUD::Housing:        targetType = BuiltType::Housing; break;
                case BuildTypeHUD::Commercial:     targetType = BuiltType::Commercial; break;
                case BuildTypeHUD::Industrial:     targetType = BuiltType::Industrial; break;
                case BuildTypeHUD::Civic:          targetType = BuiltType::Civic; break;
                case BuildTypeHUD::Cultural:       targetType = BuiltType::Cultural; break;
                default: break;
            }

            bool foundAny = false;
            for(const auto& bp : BuildingCatalog::blueprints()) {
                if(bp.builtType == targetType) {
                    if(foundAny) ImGui::SameLine();
                    foundAny = true;

                    bool isActive = (buildSystem.activeTool() == bp.tool);
                    if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, UIStyle::Colors::AmberGold);
                    if(ImGui::Button(bp.buttonLabel)) {
                        buildSystem.setActiveTool(isActive ? BuildTool::Inspect : bp.tool);
                    }
                    if(isActive) ImGui::PopStyleColor();
                }
            }

            if(!foundAny) {
                ImGui::TextDisabled("No buildings in this category yet");
            }
        }

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
