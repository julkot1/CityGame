#include "building/RoadNetwork.h"
#include "world/Chunk.h"
#include <Magnum/Math/Functions.h>
#include <set>

namespace Magnum {

UnsignedByte RoadNetwork::computeRoadMask(const World& world, Int worldX, Int worldZ) {
    UnsignedByte mask = 0;
    const struct { Int dx; Int dz; UnsignedByte bit; } neighbors[] = {
        { 0,  1, 1}, // North (+Z)
        { 1,  0, 2}, // East  (+X)
        { 0, -1, 4}, // South (-Z)
        {-1,  0, 8}  // West  (-X)
    };

    for(const auto& n : neighbors) {
        const Entity* ne = world.entityAt(worldX + n.dx, worldZ + n.dz);
        if(ne && ne->type == EntityType::RoadCobblestone) {
            mask |= n.bit;
        }
    }

    return mask;
}

void RoadNetwork::updateConnectionsAround(World& world, Int worldX, Int worldZ) {
    const Int offsets[5][2] = {{0, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    std::set<std::pair<Int, Int>> dirtyChunks;

    for(const auto& off : offsets) {
        Int nx = worldX + off[0];
        Int nz = worldZ + off[1];
        Vector2i ownerCC;
        Entity* e = world.entityAt(nx, nz, &ownerCC);
        if(e && e->type == EntityType::RoadCobblestone) {
            UnsignedByte mask = computeRoadMask(world, nx, nz);
            UnsignedInt newData = (e->data & ~0x0Fu) | (mask & 0x0Fu);
            if(e->data != newData) {
                e->data = newData;
                dirtyChunks.insert({ownerCC.x(), ownerCC.y()});
            }
        }
    }

    for(const auto& ccPair : dirtyChunks) {
        if(Chunk* ch = world.chunk({ccPair.first, ccPair.second})) {
            ch->buildMesh(world.tileSpacing());
        }
    }
}

RoadNetwork::RoadPlacementPlan RoadNetwork::evaluatePlacement(const World& world, Int worldX, Int worldZ) {
    const Tile* t = world.tileAt(worldX, worldZ);
    RoadPlacementPlan plan;
    if(!t) return plan;
    plan.roadHeight = t->height;

    const Int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for(const auto& d : dirs) {
        const Entity* ne = world.entityAt(worldX + d[0], worldZ + d[1]);
        if(ne && ne->type == EntityType::RoadCobblestone) {
            Float diff = ne->position.y() - t->height;
            if(Math::abs(diff) <= 0.10f) {
                plan.roadHeight = ne->position.y();
                break;
            } else if(diff > 0.10f && diff <= 0.65f) {
                plan.rampDirX = d[0];
                plan.rampDirZ = d[1];
                plan.deltaY = diff;
                break;
            }
        }
    }

    return plan;
}

UnsignedInt RoadNetwork::findRoadFacingOrientation(const World& world, Int baseWorldX, Int baseWorldZ,
                                                   const Vector2i& footprint,
                                                   UnsignedInt fallbackOrientation) {
    auto hasRoadAt = [&](Int wx, Int wz) -> bool {
        const Entity* e = world.entityAt(wx, wz);
        return e && e->type == EntityType::RoadCobblestone;
    };

    bool canRotate90 = (footprint.x() == footprint.y());

    // Check +Z border (orientation 0)
    for(Int dx = 0; dx < footprint.x(); ++dx) {
        if(hasRoadAt(baseWorldX + dx, baseWorldZ + footprint.y())) return 0u;
    }
    // Check +X border (orientation 1) if square
    if(canRotate90) {
        for(Int dz = 0; dz < footprint.y(); ++dz) {
            if(hasRoadAt(baseWorldX + footprint.x(), baseWorldZ + dz)) return 1u;
        }
    }
    // Check -Z border (orientation 2)
    for(Int dx = 0; dx < footprint.x(); ++dx) {
        if(hasRoadAt(baseWorldX + dx, baseWorldZ - 1)) return 2u;
    }
    // Check -X border (orientation 3) if square
    if(canRotate90) {
        for(Int dz = 0; dz < footprint.y(); ++dz) {
            if(hasRoadAt(baseWorldX - 1, baseWorldZ + dz)) return 3u;
        }
    }

    return fallbackOrientation;
}

}
