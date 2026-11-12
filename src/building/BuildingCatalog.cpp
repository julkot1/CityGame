#include "building/BuildingCatalog.h"

namespace Magnum {

const std::vector<BuildingBlueprint>& BuildingCatalog::blueprints() {
    static const std::vector<BuildingBlueprint> kBlueprints = {
        {
            BuildTool::RoadCobblestone,
            EntityType::RoadCobblestone,
            "Cobblestone Road (1x1)",
            "[1] Road",
            "Infrastructure",
            1,
            Vector2i{1, 1},
            0.65f,
            false,
            Color3{0.30f, 0.22f, 0.15f},
            false,
            false,
            true,
            BuiltType::Infrastructure
        },
        {
            BuildTool::House2x2,
            EntityType::House2x2,
            "Townhouse",
            "[2] House (2x2)",
            "Residential",
            2,
            Vector2i{2, 2},
            0.90f,
            true,
            Color3{0.28f, 0.27f, 0.26f},
            true,
            true,
            false,
            BuiltType::Housing
        },
        {
            BuildTool::HouseStandalone2x2,
            EntityType::HouseStandalone2x2,
            "Standalone Cottage",
            "[3] Cottage (2x2)",
            "Residential",
            3,
            Vector2i{2, 2},
            0.90f,
            true,
            Color3{0.28f, 0.27f, 0.26f},
            true,
            true,
            false,
            BuiltType::Housing
        },
        {
            BuildTool::CityCenter2x3,
            EntityType::CityCenter2x3,
            "City Center",
            "[4] City Center (2x3)",
            "Civic",
            4,
            Vector2i{2, 3},
            1.50f,
            true,
            Color3{0.32f, 0.31f, 0.30f},
            true,
            true,
            false,
            BuiltType::Civic
        },
        {
            BuildTool::CityPlaza4x2,
            EntityType::CityPlaza4x2,
            "City Plaza",
            "[5] Plaza (4x2)",
            "Civic",
            5,
            Vector2i{4, 2},
            1.50f,
            true,
            Color3{0.35f, 0.34f, 0.33f},
            true,
            false,
            false,
            BuiltType::Cultural
        },
        {
            BuildTool::Warehouse4x4,
            EntityType::Warehouse4x4,
            "Warehouse",
            "[6] Warehouse (4x4)",
            "Industrial",
            6,
            Vector2i{4, 4},
            1.50f,
            true,
            Color3{0.35f, 0.35f, 0.37f},
            true,
            true,
            false,
            BuiltType::Industrial
        }
    };
    return kBlueprints;
}

const BuildingBlueprint* BuildingCatalog::findByTool(BuildTool tool) {
    for(const auto& bp : blueprints()) {
        if(bp.tool == tool) return &bp;
    }
    return nullptr;
}

const BuildingBlueprint* BuildingCatalog::findByEntityType(EntityType type) {
    for(const auto& bp : blueprints()) {
        if(bp.entityType == type) return &bp;
    }
    return nullptr;
}

Vector2i BuildingCatalog::effectiveFootprint(EntityType type, UnsignedInt orientation) {
    const BuildingBlueprint* bp = findByEntityType(type);
    if(!bp) return Vector2i{1, 1};
    if((orientation & 1u) != 0u) {
        return Vector2i{bp->footprint.y(), bp->footprint.x()};
    }
    return bp->footprint;
}

Vector2 BuildingCatalog::entityHitHalfExtents(EntityType type, UnsignedInt orientation) {
    Vector2i fp = effectiveFootprint(type, orientation);
    // For 1x1: 0.45f; for 2x2: 0.95f (covers centers at +/- 0.5f from entity center)
    return Vector2{Float(fp.x()) * 0.5f - 0.05f, Float(fp.y()) * 0.5f - 0.05f};
}

}
