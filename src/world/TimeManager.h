#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include <string>

namespace Magnum {

/**
 * Manages game time progression, day-night cycle, sun/moon celestial motion,
 * and atmospheric lighting transitions (sky, sun, ambient, fog).
 */
class TimeManager {
    public:
        explicit TimeManager(Float dayDurationSeconds = 240.0f); // Default 4 minute full day

        void update(Float dt);

        // Time flow controls
        void setPaused(bool paused) { _paused = paused; }
        bool isPaused() const { return _paused; }
        void togglePaused() { _paused = !_paused; }

        void setTimeSpeed(Float multiplier) { _timeSpeed = multiplier; }
        Float timeSpeed() const { return _timeSpeed; }

        // Normalized time of day in range [0.0, 1.0)
        // 0.00 = Midnight (00:00)
        // 0.25 = Sunrise (06:00)
        // 0.50 = Noon (12:00)
        // 0.75 = Sunset (18:00)
        Float timeOfDay() const { return _timeOfDay; }
        void setTimeOfDay(Float t);

        // 24-hour clock representation
        Int hour() const;
        Int minute() const;
        std::string formattedTime() const;

        // Celestial direction & lighting properties
        Vector3 lightDirection() const { return _lightDirection; }
        Color3 lightColor() const { return _lightColor; }
        Color3 ambientColor() const { return _ambientColor; }
        Color3 skyColor() const { return _skyColor; }

        bool isNight() const { return _isNight; }

    private:
        void updateLighting();

        Float _dayDurationSeconds{240.0f};
        Float _timeOfDay{0.35f}; // Start around 08:24 AM (bright morning)
        Float _timeSpeed{1.0f};
        bool _paused{false};

        Vector3 _lightDirection{-0.4f, 0.8f, 0.45f};
        Color3 _lightColor{1.0f, 0.96f, 0.88f};
        Color3 _ambientColor{0.18f, 0.22f, 0.28f};
        Color3 _skyColor{0.65f, 0.79f, 0.92f};
        bool _isNight{false};
};

}
