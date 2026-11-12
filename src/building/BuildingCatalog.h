#pragma once

#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Color.h>
#include "world/Entity.h"

namespace Magnum {

enum class BuildTool : UnsignedShort {
    Inspect = 0,
    RoadCobblestone,
    House2x2,
    HouseStandalone2x2,
    CityCenter2x3,
    CityPlaza4x2,
    Warehouse4x4,
    Demolish
};

enum class BuiltType : UnsignedShort {
    Infrastructure = 0,     // e.g. roads
    Housing,        // e.g. houses, apartments, condos
    Commercial,     // e.g. shops, offices, factories
    Industrial,     // e.g. mines, power plants, refineries
    Civic,          // e.g. city hall, courthouse, police, fire, hospital
    Cultural,       // e.g. museums, libraries, theaters, stadiums
};
/**
 * Declarative specification for any buildable structure or infrastructure tool.
 * Adding a new building to the game only requires registering a BuildingBlueprint
 * in BuildingCatalog and a mesh builder in EntityModelRegistry.
 */
struct BuildingBlueprint {
    BuildTool tool{BuildTool::Inspect};
    EntityType entityType{EntityType::None};
    const char* name{"Unknown"};
    const char* buttonLabel{"Unknown"};
    const char* category{"General"};
    Int hotkeyNumber{0};             // e.g. 1 for [1], 2 for [2]
    Vector2i footprint{1, 1};        // Tile dimensions (width X, depth Z)
    Float maxSlope{0.45f};           // Max allowed elevation delta across footprint
    bool flattenFoundation{true};    // Auto-grade footprint tiles to a flat foundation
    Color3 foundationColor{0.28f, 0.27f, 0.26f};
    bool rotatable{false};           // Can be rotated with [R]
    bool autoAlignToRoad{false};     // Automatically faces adjacent cobblestone road
    bool isRoadNetwork{false};       // Uses RoadNetwork auto-tiling & stepped ramps
    BuiltType builtType{BuiltType::Infrastructure}; // Category for city simulation logic (e.g. housing, civic, etc.)
};

class BuildingCatalog {
    public:
        static const std::vector<BuildingBlueprint>& blueprints();
        static const BuildingBlueprint* findByTool(BuildTool tool);
        static const BuildingBlueprint* findByEntityType(EntityType type);

        // Returns the effective (X, Z) tile footprint for an entity, accounting for 90-degree rotation
        static Vector2i effectiveFootprint(EntityType type, UnsignedInt orientation = 0);

        // Returns half-extents (radiusX, radiusZ) in tile space for hit-testing / collision lookup
        static Vector2 entityHitHalfExtents(EntityType type, UnsignedInt orientation = 0);
};

}
