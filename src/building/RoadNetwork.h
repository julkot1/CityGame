#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/World.h"

namespace Magnum {

/**
 * Modular helper responsible for road connectivity bitmasks, neighbor updates,
 * micro-irregularity terrain leveling, and stepped ramp grading.
 */
class RoadNetwork {
    public:
        struct RoadPlacementPlan {
            Float roadHeight{0.0f};
            Float deltaY{0.0f};
            Int rampDirX{0};
            Int rampDirZ{0};
        };

        // Computes 4-neighbor road bitmask in world coordinates: North(+Z)=1, East(+X)=2, South(-Z)=4, West(-X)=8
        static UnsignedByte computeRoadMask(const World& world, Int worldX, Int worldZ);

        // Updates the connectivity mask of the road at (worldX, worldZ) and its 4 cardinal neighbors (even across chunks)
        static void updateConnectionsAround(World& world, Int worldX, Int worldZ);

        // Evaluates terrain leveling and stepped ramp parameters in world coordinates
        static RoadPlacementPlan evaluatePlacement(const World& world, Int worldX, Int worldZ);

        // Finds the orientation (0=+Z, 1=+X, 2=-Z, 3=-X) facing an adjacent road bordering an NxM plot in world coords
        static UnsignedInt findRoadFacingOrientation(const World& world, Int baseWorldX, Int baseWorldZ,
                                                     const Vector2i& footprint,
                                                     UnsignedInt fallbackOrientation);
};

}
