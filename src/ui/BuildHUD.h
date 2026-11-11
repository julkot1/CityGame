#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/BuildSystem.h"

namespace Magnum {

/**
 * Bottom-right HUD: Active build tool indicator, road/house/demolish buttons, placement status feedback.
 */
class BuildHUD {
    public:
        explicit BuildHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize, BuildSystem& buildSystem, const PlacementValidation& validation);
};

}
