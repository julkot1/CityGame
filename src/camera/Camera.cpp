#include "camera/Camera.h"

#include <cmath>
#include <Magnum/Math/Functions.h>
#include <algorithm>

namespace Magnum {

using namespace Math::Literals;

Camera::Camera(const Vector2i& viewportSize):
    _pitch{35.0_degf}
{
    setViewport(viewportSize);
}

void Camera::setViewport(const Vector2i& viewportSize) {
    if(viewportSize.x() > 0 && viewportSize.y() > 0) {
        _projection = Matrix4::perspectiveProjection(
            45.0_degf,
            Vector2{viewportSize}.aspectRatio(),
            0.1f,
            1000.0f
        );
    }
}

void Camera::clampTarget(const Vector3& minBounds, const Vector3& maxBounds) {
    _target.x() = Math::clamp(_target.x(), minBounds.x(), maxBounds.x());
    _target.z() = Math::clamp(_target.z(), minBounds.z(), maxBounds.z());
}

void Camera::move(const Vector3& offset) {
    _target += offset;
}

void Camera::rotate(Rad deltaYaw, Rad deltaPitch) {
    _yaw += deltaYaw;
    _pitch = Math::clamp(Rad(_pitch + deltaPitch), Rad(10.0_degf), Rad(85.0_degf));
}

void Camera::zoom(Float delta) {
    _distance = Math::clamp(_distance - delta * 1.5f, 2.0f, 120.0f);
}

Vector3 Camera::forward() const {
    return Vector3{-std::sin(Float(_yaw)), 0.0f, -std::cos(Float(_yaw))};
}

Vector3 Camera::right() const {
    return Vector3{std::cos(Float(_yaw)), 0.0f, -std::sin(Float(_yaw))};
}

Matrix4 Camera::viewMatrix() const {
    Float r = _distance;
    Float pitch = Float(_pitch);
    Float yaw = Float(_yaw);

    Vector3 eye{
        _target.x() + r * std::cos(pitch) * std::sin(yaw),
        _target.y() + r * std::sin(pitch),
        _target.z() + r * std::cos(pitch) * std::cos(yaw)
    };

    return Matrix4::lookAt(eye, _target, Vector3::yAxis()).invertedRigid();
}

}
