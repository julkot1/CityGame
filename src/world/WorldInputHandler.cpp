#include "world/WorldInputHandler.h"
#include <Corrade/Utility/Debug.h>

namespace Magnum {

WorldInputHandler::WorldInputHandler(World& world, const Camera& camera):
    _world{world},
    _camera{camera}
{
    _onTileLeftClick = [](const World::RaycastHit& hit, Input::Pointer) {
        Debug{} << "[Left Click] Tile at world (" << hit.worldTileX << "," << hit.worldTileZ << ")"
                << "| chunk (" << hit.chunkCoord.x() << "," << hit.chunkCoord.y() << ")"
                << "| local (" << hit.localTileCoord.x() << "," << hit.localTileCoord.y() << ")";
    };

    _onTileRightClick = [](const World::RaycastHit& hit, Input::Pointer) {
        Debug{} << "[Right Click] Tile at world (" << hit.worldTileX << "," << hit.worldTileZ << ")"
                << "| chunk (" << hit.chunkCoord.x() << "," << hit.chunkCoord.y() << ")"
                << "| local (" << hit.localTileCoord.x() << "," << hit.localTileCoord.y() << ")";
    };
}

void WorldInputHandler::update(const Input& input, const Vector2i& viewportSize) {
    Vector3 rayDir = _camera.unprojectRay(input.cursorPosition(), viewportSize);
    _hoverHit = _world.raycastTile(_camera.eyePosition(), rayDir);

    if(_hoverHit.hit && _onTileHover) {
        _onTileHover(_hoverHit);
    }

    if(_hoverHit.hit) {
        if(input.isPointerJustPressed(Input::Pointer::MouseLeft) && _onTileLeftClick) {
            _onTileLeftClick(_hoverHit, Input::Pointer::MouseLeft);
        }
        if(input.isPointerJustPressed(Input::Pointer::MouseRight) && _onTileRightClick) {
            _onTileRightClick(_hoverHit, Input::Pointer::MouseRight);
        }
    }
}

}
