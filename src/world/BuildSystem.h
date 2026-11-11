#pragma once

#include <string>
#include <Magnum/Magnum.h>
#include "world/World.h"
#include "world/Entity.h"

namespace Magnum {

enum class BuildTool : UnsignedShort {
    Inspect = 0,
    RoadCobblestone,
    Demolish
};

struct PlacementValidation {
    bool valid{false};
    std::string reason{""};
};

/**
 * Manages active construction tools, placement validation rules, and world mutation.
 */
class BuildSystem {
    public:
        explicit BuildSystem();

        BuildTool activeTool() const { return _activeTool; }
        void setActiveTool(BuildTool tool) { _activeTool = tool; }
        bool isBuilding() const { return _activeTool != BuildTool::Inspect; }

        PlacementValidation validatePlacement(const World& world, const World::RaycastHit& hit) const;
        bool executeAction(World& world, const World::RaycastHit& hit);

        const char* toolName(BuildTool tool) const;

        // Auto-tiling road connectivity calculation (N=1, E=2, S=4, W=8)
        static UnsignedByte computeRoadMask(const World& world, const Vector2i& chunkCoord, Int localX, Int localZ);
        static void updateRoadConnectionsAround(World& world, const Vector2i& chunkCoord, Int localX, Int localZ);

    private:
        BuildTool _activeTool{BuildTool::Inspect};
        UnsignedInt _nextEntityId{10000};
};

}
