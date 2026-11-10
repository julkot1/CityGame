#pragma once

#include <imgui.h>

namespace Magnum {

/**
 * Manages Dear ImGui styling, color palette, spacing, and theme configuration.
 */
class UIStyle {
    public:
        static void apply();

        // Predefined color palette constants for consistent UI elements
        struct Colors {
            static constexpr ImVec4 AmberGold{0.95f, 0.75f, 0.22f, 1.0f};
            static constexpr ImVec4 NeonGreen{0.35f, 0.90f, 0.40f, 1.0f};
            static constexpr ImVec4 SoftRed{0.95f, 0.35f, 0.35f, 1.0f};
            static constexpr ImVec4 SkyBlue{0.45f, 0.65f, 1.00f, 1.0f};
            static constexpr ImVec4 TextMuted{0.52f, 0.58f, 0.66f, 1.0f};
            static constexpr ImVec4 TextBright{0.96f, 0.97f, 0.99f, 1.0f};
        };
};

}
