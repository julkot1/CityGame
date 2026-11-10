#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>

namespace Magnum {

/**
 * Handles in-game HUD overlay rendering (FPS counter, stats, tools, etc.).
 */
class GameHUD {
    public:
        explicit GameHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize, UnsignedInt chunkCount);

        Float currentFps() const { return _currentFps; }
        Float currentFrameTimeMs() const { return _currentFrameTimeMs; }

    private:
        UnsignedInt _frameCount{0};
        Float _timeAccumulator{0.0f};
        Float _currentFps{0.0f};
        Float _currentFrameTimeMs{0.0f};
};

}
