#pragma once

#include <string>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

struct TimeLightingSnapshot {
    Float timeOfDay{0.50f};
    Int hour{12};
    Int minute{0};
    std::string formattedTime{"12:00"};
    bool isPaused{false};
    Float timeSpeed{1.0f};

    Vector3 lightDirection{-0.4f, 0.8f, -0.4f};
    Color3 lightColor{1.0f, 0.92f, 0.78f};
    Color3 ambientColor{0.20f, 0.26f, 0.38f};
    Color3 skyColor{0.65f, 0.79f, 0.92f};
    bool isNight{false};
};

}
