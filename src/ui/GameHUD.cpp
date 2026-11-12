#include "ui/GameHUD.h"

namespace Magnum {

GameHUD::GameHUD() = default;

void GameHUD::update(Float dt) {
    _performanceHUD.update(dt);
}

void GameHUD::draw(const Vector2i& windowSize,
                    UnsignedInt renderedChunks,
                    UnsignedInt totalChunks,
                    TimeManager* timeManager,
                    const World* world,
                    const World::RaycastHit& selectedHit,
                    BuildSystem* buildSystem,
                    const PlacementValidation& validation,
                    Float simTPS,
                    Float simTickDurationMs) {
    // 1. Top-Left: Performance & culling stats
    _performanceHUD.draw(windowSize, renderedChunks, totalChunks, simTPS, simTickDurationMs);

    // 2. Top-Right: Celestial clock & time speed controls
    if(timeManager) {
        _timeHUD.draw(windowSize, *timeManager);
    }

    // 3. Bottom-Left: Selected tile inspection card
    if(world && selectedHit.hit) {
        _tileInspectorHUD.draw(windowSize, *world, selectedHit);
    }

    // 4. Bottom-Right: Construction toolbar & status
    if(buildSystem) {
        _buildHUD.draw(windowSize, *buildSystem, validation);
    }
}

}
