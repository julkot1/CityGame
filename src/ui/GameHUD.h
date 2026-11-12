#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "ui/PerformanceHUD.h"
#include "ui/BuildHUD.h"
#include "ui/TimeHUD.h"
#include "ui/TileInspectorHUD.h"
#include "world/TimeManager.h"
#include "world/World.h"
#include "building/BuildSystem.h"

namespace Magnum {

/**
 * High-level HUD coordinator that aggregates and manages individual HUD widgets.
 */
class GameHUD {
    public:
        explicit GameHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize,
                  UnsignedInt renderedChunks,
                  UnsignedInt totalChunks,
                  TimeManager* timeManager = nullptr,
                  const World* world = nullptr,
                  const World::RaycastHit& selectedHit = {},
                  BuildSystem* buildSystem = nullptr,
                  const PlacementValidation& validation = {},
                  Float simTPS = 60.0f,
                  Float simTickDurationMs = 0.0f);

        PerformanceHUD& performanceHUD() { return _performanceHUD; }
        TimeHUD& timeHUD() { return _timeHUD; }
        TileInspectorHUD& tileInspectorHUD() { return _tileInspectorHUD; }
        BuildHUD& buildHUD() { return _buildHUD; }

    private:
        PerformanceHUD _performanceHUD;
        TimeHUD _timeHUD;
        TileInspectorHUD _tileInspectorHUD;
        BuildHUD _buildHUD;
};

}
