#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <string>
namespace Magnum {

/**
 * Top-left HUD widget: FPS counter, frame time, chunk rendering and culling stats.
 */
class BuildHUD {
    public:
        explicit BuildHUD();

        void update(Float dt);
        void draw(const Vector2i& windowSize);
    private:
        std::string selected{"None"};

};

}
