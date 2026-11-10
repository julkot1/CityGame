#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Vector2.h>

namespace Magnum {

class Camera {
    public:
        explicit Camera(const Vector2i& viewportSize);

        void setViewport(const Vector2i& viewportSize);
        void move(const Vector3& offset);
        void setTarget(const Vector3& target) { _target = target; }
        void clampTarget(const Vector3& minBounds, const Vector3& maxBounds);
        void rotate(Rad deltaYaw, Rad deltaPitch);
        void zoom(Float delta);

        Vector3 forward() const;
        Vector3 right() const;

        Matrix4 projectionMatrix() const { return _projection; }
        Matrix4 viewMatrix() const;
        Matrix4 viewProjectionMatrix() const { return _projection * viewMatrix(); }

        Vector3 target() const { return _target; }
        Rad yaw() const { return _yaw; }
        Rad pitch() const { return _pitch; }
        Float distance() const { return _distance; }
    private:
        Matrix4 _projection;
        Vector3 _target{0.0f, 0.0f, 0.0f};
        Rad _yaw{0.0f};
        Rad _pitch{0.0f};
        Float _distance{45.0f};
};

}
