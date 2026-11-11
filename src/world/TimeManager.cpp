#include "world/TimeManager.h"
#include <Magnum/Math/Functions.h>
#include <Magnum/Math/Color.h>
#include <Corrade/Utility/FormatStl.h>
#include <cmath>

namespace Magnum {

using namespace Math::Literals;

TimeManager::TimeManager(Float dayDurationSeconds):
    _dayDurationSeconds{Math::max(10.0f, dayDurationSeconds)}
{
    updateLighting();
}

void TimeManager::setTimeOfDay(Float t) {
    _timeOfDay = std::fmod(t, 1.0f);
    if(_timeOfDay < 0.0f) _timeOfDay += 1.0f;
    updateLighting();
}

void TimeManager::update(Float dt) {
    if(_paused) return;

    Float progress = (dt * _timeSpeed) / _dayDurationSeconds;
    _timeOfDay = std::fmod(_timeOfDay + progress, 1.0f);
    if(_timeOfDay < 0.0f) _timeOfDay += 1.0f;

    updateLighting();
}

Int TimeManager::hour() const {
    Float totalMinutes = _timeOfDay * 1440.0f; // 24 * 60
    return Int(totalMinutes / 60.0f) % 24;
}

Int TimeManager::minute() const {
    Float totalMinutes = _timeOfDay * 1440.0f;
    return Int(std::fmod(totalMinutes, 60.0f));
}

std::string TimeManager::formattedTime() const {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", hour(), minute());
    return std::string{buf};
}

void TimeManager::updateLighting() {
    // 0.0 = midnight, 0.25 = 06:00 (sunrise), 0.5 = 12:00 (noon), 0.75 = 18:00 (sunset)
    // Angle in radians: at 0.5 (noon), angle = pi / 2 (sin = 1.0, high in sky)
    // At 0.0 (midnight), angle = -pi / 2 (sin = -1.0, below horizon)
    Float sunAngle = (_timeOfDay - 0.25f) * Constants::pi() * 2.0f;

    Float sunY = Math::sin(Rad{sunAngle});
    Float sunX = -Math::cos(Rad{sunAngle});
    Float sunZ = 0.35f; // Slight south-facing inclination

    Vector3 rawSunDir = Vector3{sunX, sunY, sunZ}.normalized();
    _isNight = (sunY < 0.0f);

    // If sun is below horizon, light comes from the Moon
    if(!_isNight) {
        _lightDirection = rawSunDir;
    } else {
        // Moon rises on opposite side of sky with minimum elevation so shadows still function
        Vector3 rawMoonDir = Vector3{-sunX, -sunY, -sunZ}.normalized();
        _lightDirection = Vector3{rawMoonDir.x(), Math::max(rawMoonDir.y(), 0.30f), rawMoonDir.z()}.normalized();
    }

    // Atmospheric transition palettes (19th-Century Victorian / Steampunk mood):
    // 1. Midday (Warm brass sunlight & hazy parchment-slate sky)
    const Color3 daySunColor{1.05f, 0.92f, 0.76f};
    const Color3 dayAmbientColor{0.19f, 0.21f, 0.25f};
    const Color3 daySkyColor{0.64f, 0.69f, 0.73f};

    // 2. Sunrise / Sunset (Smoky copper-amber industrial Golden Hour)
    const Color3 sunsetSunColor{1.10f, 0.50f, 0.20f};
    const Color3 sunsetAmbientColor{0.22f, 0.15f, 0.17f};
    const Color3 sunsetSkyColor{0.74f, 0.48f, 0.34f};

    // 3. Night (Victorian gaslight & Prussian-blue moonlight)
    const Color3 nightMoonColor{0.18f, 0.24f, 0.36f};
    const Color3 nightAmbientColor{0.05f, 0.06f, 0.09f};
    const Color3 nightSkyColor{0.04f, 0.06f, 0.10f};

    // Blend weights based on sun elevation angle sunY (-1.0 to 1.0)
    if(sunY > 0.25f) {
        // High daytime: blend from sunset to full midday
        Float t = Math::clamp((sunY - 0.25f) / 0.50f, 0.0f, 1.0f);
        _lightColor = Math::lerp(sunsetSunColor, daySunColor, t);
        _ambientColor = Math::lerp(sunsetAmbientColor, dayAmbientColor, t);
        _skyColor = Math::lerp(sunsetSkyColor, daySkyColor, t);
    } else if(sunY >= -0.10f) {
        // Horizon crossing (Dawn / Dusk): blend between sunset and night
        Float t = Math::clamp((sunY - (-0.10f)) / 0.35f, 0.0f, 1.0f);
        _lightColor = Math::lerp(nightMoonColor, sunsetSunColor, t);
        _ambientColor = Math::lerp(nightAmbientColor, sunsetAmbientColor, t);
        _skyColor = Math::lerp(nightSkyColor, sunsetSkyColor, t);
    } else {
        // Deep Night
        Float t = Math::clamp((-sunY - 0.10f) / 0.50f, 0.0f, 1.0f);
        _lightColor = nightMoonColor * (1.0f - 0.25f * t);
        _ambientColor = nightAmbientColor;
        _skyColor = nightSkyColor;
    }
}

}
