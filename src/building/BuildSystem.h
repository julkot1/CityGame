#pragma once

#include <string>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/World.h"
#include "world/Entity.h"
#include "building/BuildingCatalog.h"

namespace Magnum {

struct PlacementValidation {
    bool valid{false};
    std::string reason{""};
};

/**
 * Modular, blueprint-driven construction controller.
 * Delegates structure specs to BuildingCatalog and road connectivity to RoadNetwork.
 */
class BuildSystem {
    public:
        explicit BuildSystem();

        BuildTool activeTool() const { return _activeTool; }
        void setActiveTool(BuildTool tool) { _activeTool = tool; }
        bool isBuilding() const { return _activeTool != BuildTool::Inspect; }

        const BuildingBlueprint* activeBlueprint() const {
            return BuildingCatalog::findByTool(_activeTool);
        }

        UnsignedInt buildingOrientation() const { return _buildingOrientation; }
        void rotateBuilding() { _buildingOrientation = (_buildingOrientation + 1u) & 3u; }

        // Returns the effective footprint (accounting for rotation) of the currently selected tool
        Vector2i toolFootprint() const;

        PlacementValidation validatePlacement(const World& world, const World::RaycastHit& hit) const;
        bool executeAction(World& world, const World::RaycastHit& hit);

        const char* toolName(BuildTool tool) const;

    private:
        PlacementValidation validateBlueprintPlacement(const World& world, Int worldX, Int worldZ,
                                                       const BuildingBlueprint& bp) const;
        bool executeDemolish(World& world, Int worldX, Int worldZ);
        bool executeRoadPlacement(World& world, const World::RaycastHit& hit,
                                  const BuildingBlueprint& bp);
        bool executeStructurePlacement(World& world, const World::RaycastHit& hit,
                                       const BuildingBlueprint& bp);

        BuildTool _activeTool{BuildTool::Inspect};
        UnsignedInt _buildingOrientation{0}; // 0 = +Z, 1 = +X, 2 = -Z, 3 = -X
        UnsignedInt _nextEntityId{10000};
};

}
