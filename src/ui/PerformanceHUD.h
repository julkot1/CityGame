#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>

namespace Magnum {

/**
 * Top-left HUD widget: FPS counter, frame time, chunk rendering and culling stats.
 */
class PerformanceHUD {
    public:
        explicit PerformanceHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize, UnsignedInt renderedChunks, UnsignedInt totalChunks,
                  Float simTPS = 60.0f, Float simTickDurationMs = 0.0f);

        Float currentFps() const { return _currentFps; }
        Float currentFrameTimeMs() const { return _currentFrameTimeMs; }

    private:
        UnsignedInt _frameCount{0};
        Float _timeAccumulator{0.0f};
        Float _currentFps{0.0f};
        Float _currentFrameTimeMs{0.0f};
};

}
