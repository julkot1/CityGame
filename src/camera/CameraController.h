#pragma once

#include <Magnum/Magnum.h>
#include "camera/Camera.h"
#include "input/Input.h"

namespace Magnum {

class CameraController {
    public:
        explicit CameraController(Camera& camera, Float panSpeed = 6.0f, Float rotateSpeed = 1.5f);

        void update(const Input& input, Float dt);

        void setPanSpeed(Float speed) { _panSpeed = speed; }
        Float panSpeed() const { return _panSpeed; }

        void setRotateSpeed(Float speed) { _rotateSpeed = speed; }
        Float rotateSpeed() const { return _rotateSpeed; }

    private:
        Camera& _camera;
        Float _panSpeed{6.0f};
        Float _rotateSpeed{1.5f};
};

}
