#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/TimeManager.h"

namespace Magnum {

/**
 * Top-right HUD widget: In-game clock, celestial phase, play/pause, time speed presets, scrub slider.
 */
class TimeHUD {
    public:
        explicit TimeHUD();

        void draw(const Vector2i& windowSize, TimeManager& timeManager);
};

}
