#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "building/BuildSystem.h"

namespace Magnum {

/**
 * Bottom-right HUD: Active build tool indicator, road/house/demolish buttons, placement status feedback.
 */
enum class BuildTypeHUD : UnsignedShort {
    Infrastructure = 0,     // e.g. roads
    Housing,        // e.g. houses, apartments, condos
    Commercial,     // e.g. shops, offices, factories
    Industrial,     // e.g. mines, power plants, refineries
    Civic,          // e.g. city hall, courthouse, police, fire, hospital
    Cultural,       // e.g. museums, libraries, theaters, stadiums
    None            // No specific type (default)
};
class BuildHUD {
    private:
        BuildTypeHUD _activeBuiltType{BuildTypeHUD::None};
    public:
        explicit BuildHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize, BuildSystem& buildSystem, const PlacementValidation& validation);
};

}
