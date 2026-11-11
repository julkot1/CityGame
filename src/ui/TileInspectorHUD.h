#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/World.h"

namespace Magnum {

/**
 * Bottom-left HUD widget: Displays detailed survey statistics for the currently selected tile.
 */
class TileInspectorHUD {
    public:
        explicit TileInspectorHUD();

        void draw(const Vector2i& windowSize, const World& world, const World::RaycastHit& selectedHit);
};

}
