#pragma once

#include <functional>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include "world/World.h"
#include "camera/Camera.h"
#include "input/Input.h"

namespace Magnum {

class WorldInputHandler {
    public:
        using TileActionCallback = std::function<void(const World::RaycastHit& hit, Input::Pointer button)>;

        explicit WorldInputHandler(World& world, const Camera& camera);

        void update(const Input& input, const Vector2i& viewportSize);

        const World::RaycastHit& hoverHit() const { return _hoverHit; }
        bool isHoveringTile() const { return _hoverHit.hit; }

        const World::RaycastHit& selectedHit() const { return _selectedHit; }
        bool hasSelection() const { return _selectedHit.hit; }
        void clearSelection() { _selectedHit = {}; }
        void selectTile(const World::RaycastHit& hit) { _selectedHit = hit; }

        void setOnTileLeftClick(TileActionCallback callback) { _onTileLeftClick = std::move(callback); }
        void setOnTileRightClick(TileActionCallback callback) { _onTileRightClick = std::move(callback); }
        void setOnTileHover(std::function<void(const World::RaycastHit&)> callback) { _onTileHover = std::move(callback); }

    private:
        World& _world;
        const Camera& _camera;

        World::RaycastHit _hoverHit;
        World::RaycastHit _selectedHit;
        TileActionCallback _onTileLeftClick;
        TileActionCallback _onTileRightClick;
        std::function<void(const World::RaycastHit&)> _onTileHover;
};

}
