#include "ui/UIStyle.h"

namespace Magnum {

void UIStyle::apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Modern Sci-Fi / Game Studio Dark Frosted Glass Theme ---
    // Deep carbon background with high-end glass transparency
    colors[ImGuiCol_WindowBg]             = ImVec4(0.05f, 0.06f, 0.09f, 0.88f); // Deep carbon glass
    colors[ImGuiCol_ChildBg]              = ImVec4(0.08f, 0.10f, 0.14f, 0.60f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.06f, 0.07f, 0.11f, 0.96f);

    // Subtle neon border accent
    colors[ImGuiCol_Border]               = ImVec4(0.20f, 0.32f, 0.48f, 0.55f); // Cyber blue border
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Typography
    colors[ImGuiCol_Text]                 = Colors::TextBright;
    colors[ImGuiCol_TextDisabled]         = Colors::TextMuted;

    // Title Bars: Clean, integrated into the panel
    colors[ImGuiCol_TitleBg]              = ImVec4(0.07f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.10f, 0.14f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.04f, 0.05f, 0.08f, 0.60f);

    // Modern Flat Buttons with glowing hover
    colors[ImGuiCol_Button]               = ImVec4(0.11f, 0.16f, 0.24f, 0.85f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.18f, 0.30f, 0.46f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.95f, 0.72f, 0.18f, 0.95f); // Amber flash on click

    // Input fields & Frames (sharp contrast with inset feel)
    colors[ImGuiCol_FrameBg]              = ImVec4(0.08f, 0.11f, 0.17f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.14f, 0.20f, 0.30f, 0.90f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.26f, 0.38f, 1.00f);

    // Sliders & Tabs (Electric Amber accent)
    colors[ImGuiCol_SliderGrab]           = Colors::AmberGold;
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(1.00f, 0.85f, 0.30f, 1.00f);
    colors[ImGuiCol_CheckMark]            = Colors::AmberGold;

    // Header selections (dropdown items, trees)
    colors[ImGuiCol_Header]               = ImVec4(0.14f, 0.22f, 0.34f, 0.80f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.20f, 0.32f, 0.48f, 0.90f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.24f, 0.38f, 0.58f, 1.00f);

    // Separators
    colors[ImGuiCol_Separator]            = ImVec4(0.18f, 0.26f, 0.38f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]     = Colors::AmberGold;
    colors[ImGuiCol_SeparatorActive]      = Colors::AmberGold;

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.04f, 0.05f, 0.08f, 0.30f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.18f, 0.26f, 0.38f, 0.70f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.40f, 0.58f, 0.90f);
    colors[ImGuiCol_ScrollbarGrabActive]  = Colors::AmberGold;

    // --- Precise Geometric Layout & Generous Padding ---
    style.WindowPadding     = ImVec2(18.0f, 16.0f);
    style.FramePadding      = ImVec2(12.0f, 8.0f);   // Taller, touch/click-friendly controls
    style.ItemSpacing       = ImVec2(12.0f, 10.0f);  // Clean breathing room between elements
    style.ItemInnerSpacing  = ImVec2(10.0f, 6.0f);

    // Modern chamfered/rounded corners
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 5.0f;
    style.PopupRounding     = 6.0f;
    style.FrameRounding     = 4.0f; // Sharp, techy buttons
    style.GrabRounding      = 4.0f;
    style.ScrollbarRounding = 8.0f;
    style.TabRounding       = 4.0f;

    // Clean single-pixel hairline borders
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
}

}
