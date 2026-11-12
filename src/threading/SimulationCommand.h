#pragma once

#include <string>
#include <Magnum/Magnum.h>
#include "building/BuildingCatalog.h"
#include "ui/MenuManager.h"

namespace Magnum {

enum class SimulationCommandType {
    PlaceStructure,
    Demolish,
    TimeTogglePause,
    TimeSetSpeed,
    TimeSetTimeOfDay,
    CreateNewWorld,
    SaveWorld
};

struct SimulationCommand {
    SimulationCommandType type{SimulationCommandType::PlaceStructure};

    // For PlaceStructure / Demolish
    BuildTool tool{BuildTool::Inspect};
    Int worldX{0};
    Int worldZ{0};
    UnsignedInt orientation{0};

    // For Time commands
    Float timeValue{0.0f};

    // For World operations
    WorldCreationParams worldParams;
    std::string filename;
};

}
