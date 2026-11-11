#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

namespace Magnum {

enum class EntityType : UnsignedShort {
    None = 0,
    TreeOak,
    TreePine,
    Rock,
    RoadCobblestone,
    CoalDeposit,
    IronDeposit,
    CopperDeposit,
    AncientRuins
};

struct Entity {
    UnsignedInt id{0};
    EntityType type{EntityType::None};
    Vector3 position{0.0f}; // Position relative to chunk origin (X, Y, Z)
    Float rotationY{0.0f};  // Y-axis rotation in radians
    Float scale{1.0f};      // Scaling factor
    UnsignedInt data{0};    // Custom metadata (e.g. ramp direction / height delta)
};

}
