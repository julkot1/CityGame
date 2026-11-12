#include "building/BuildSystem.h"
#include "building/RoadNetwork.h"
#include "world/Chunk.h"
#include <cmath>
#include <set>

namespace Magnum {

BuildSystem::BuildSystem() = default;

const char* BuildSystem::toolName(BuildTool tool) const {
    if(tool == BuildTool::Inspect) return "Inspect & Survey";
    if(tool == BuildTool::Demolish) return "Demolish / Bulldoze";
    if(const BuildingBlueprint* bp = BuildingCatalog::findByTool(tool)) {
        return bp->name;
    }
    return "Unknown";
}

Vector2i BuildSystem::toolFootprint() const {
    if(const BuildingBlueprint* bp = activeBlueprint()) {
        return BuildingCatalog::effectiveFootprint(bp->entityType, _buildingOrientation);
    }
    return Vector2i{1, 1};
}

PlacementValidation BuildSystem::validatePlacement(const World& world, const World::RaycastHit& hit) const {
    if(!hit.hit) {
        return {false, "No tile selected"};
    }

    Int wx = hit.worldTileX;
    Int wz = hit.worldTileZ;

    if(_activeTool == BuildTool::Demolish) {
        if(!world.entityAt(wx, wz)) {
            return {false, "No entity on this tile to demolish"};
        }
        return {true, "Ready to bulldoze"};
    }

    if(const BuildingBlueprint* bp = activeBlueprint()) {
        return validateBlueprintPlacement(world, wx, wz, *bp);
    }

    return {true, "OK"};
}

PlacementValidation BuildSystem::validateBlueprintPlacement(const World& world, Int worldX, Int worldZ,
                                                            const BuildingBlueprint& bp) const {
    Vector2i fp = BuildingCatalog::effectiveFootprint(bp.entityType, _buildingOrientation);

    Float minH = 1e9f;
    Float maxH = -1e9f;

    // 1. Validate every tile inside the NxM footprint across chunk boundaries
    for(Int dz = 0; dz < fp.y(); ++dz) {
        for(Int dx = 0; dx < fp.x(); ++dx) {
            Int tx = worldX + dx;
            Int tz = worldZ + dz;
            const Tile* ft = world.tileAt(tx, tz);
            if(!ft || !ft->active) {
                return {false, "Plot extends outside world map boundary"};
            }
            if(ft->type == TileType::Water) {
                return {false, "Cannot build on water"};
            }
            if(ft->type == TileType::Snow && !bp.isRoadNetwork) {
                return {false, "Cannot build structure on alpine frost peak"};
            }
            const Entity* occ = world.entityAt(tx, tz);
            if(occ) {
                if(bp.isRoadNetwork && occ->type == bp.entityType) {
                    return {false, "Road already exists here"};
                }
                // Allow multi-tile buildings to auto-clear natural trees & rocks on their footprint
                bool isAutoClearableFlora = !bp.isRoadNetwork &&
                    (occ->type == EntityType::TreeOak ||
                     occ->type == EntityType::TreePine ||
                     occ->type == EntityType::Rock);
                if(!isAutoClearableFlora) {
                    return {false, "Plot obstructed (clear existing structure/road first)"};
                }
            }
            minH = Math::min(minH, ft->height);
            maxH = Math::max(maxH, ft->height);
        }
    }

    // 2. Road-specific adjacent slope check vs Multi-tile foundation slope check
    if(bp.isRoadNetwork) {
        const Tile* tile = world.tileAt(worldX, worldZ);
        const Int neighbors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for(const auto& n : neighbors) {
            const Entity* ne = world.entityAt(worldX + n[0], worldZ + n[1]);
            if(ne && ne->type == EntityType::RoadCobblestone) {
                if(Math::abs(ne->position.y() - tile->height) > bp.maxSlope) {
                    return {false, "Cliff too steep to grade road (max 0.6m)"};
                }
            }
        }
    } else if(maxH - minH > bp.maxSlope) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Slope too steep (%.2fm > max %.2fm)", static_cast<double>(maxH - minH), static_cast<double>(bp.maxSlope));
        return {false, buf};
    }

    return {true, std::string{"Ready to construct "} + bp.name};
}

bool BuildSystem::executeAction(World& world, const World::RaycastHit& hit, std::vector<ChunkMeshData>* outDirtyMeshes) {
    PlacementValidation val = validatePlacement(world, hit);
    if(!val.valid) {
        return false;
    }

    if(_activeTool == BuildTool::Demolish) {
        return executeDemolish(world, hit.worldTileX, hit.worldTileZ, outDirtyMeshes);
    }

    if(const BuildingBlueprint* bp = activeBlueprint()) {
        if(bp->isRoadNetwork) {
            return executeRoadPlacement(world, hit, *bp, outDirtyMeshes);
        }
        return executeStructurePlacement(world, hit, *bp, outDirtyMeshes);
    }

    return false;
}

bool BuildSystem::executeDemolish(World& world, Int worldX, Int worldZ, std::vector<ChunkMeshData>* outDirtyMeshes) {
    Entity removedEntity;
    Vector2i ownerCC;
    if(!world.removeEntityAt(worldX, worldZ, &removedEntity, &ownerCC)) {
        return false;
    }

    std::set<std::pair<Int, Int>> dirtyChunks;
    dirtyChunks.insert({ownerCC.x(), ownerCC.y()});

    // Compute world coordinates of the removed entity's footprint origin
    Vector2i fp = BuildingCatalog::effectiveFootprint(removedEntity.type, removedEntity.data & 3u);
    Float worldEntX = Float(ownerCC.x() * Chunk::Size) + removedEntity.position.x();
    Float worldEntZ = Float(ownerCC.y() * Chunk::Size) + removedEntity.position.z();
    Int baseWorldX = Int(std::round(worldEntX - Float(fp.x() - 1) * 0.5f));
    Int baseWorldZ = Int(std::round(worldEntZ - Float(fp.y() - 1) * 0.5f));

    for(Int dz = 0; dz < fp.y(); ++dz) {
        for(Int dx = 0; dx < fp.x(); ++dx) {
            Int tx = baseWorldX + dx;
            Int tz = baseWorldZ + dz;
            if(Tile* ft = world.tileAt(tx, tz)) {
                if(ft->type == TileType::Road) {
                    ft->type = TileType::Grass;
                    ft->color = Color3{0x566944_rgbf};
                }
                Vector2i cc = World::worldToChunkCoord(tx, tz);
                dirtyChunks.insert({cc.x(), cc.y()});
            }
        }
    }

    for(const auto& ccPair : dirtyChunks) {
        if(Chunk* ch = world.chunk({ccPair.first, ccPair.second})) {
            if(outDirtyMeshes) {
                outDirtyMeshes->push_back(ch->buildCpuMesh(world.tileSpacing()));
            }
        }
    }

    RoadNetwork::updateConnectionsAround(world, worldX, worldZ, outDirtyMeshes);
    return true;
}

bool BuildSystem::executeRoadPlacement(World& world, const World::RaycastHit& hit,
                                       const BuildingBlueprint& bp, std::vector<ChunkMeshData>* outDirtyMeshes) {
    Chunk* chunk = world.chunk(hit.chunkCoord);
    if(!chunk) return false;

    Int wx = hit.worldTileX;
    Int wz = hit.worldTileZ;
    Int lx = hit.localTileCoord.x();
    Int lz = hit.localTileCoord.y();

    RoadNetwork::RoadPlacementPlan plan = RoadNetwork::evaluatePlacement(world, wx, wz);

    Tile& t = chunk->tile(lx, lz);
    t.height = plan.roadHeight;
    t.type = TileType::Road;
    t.color = bp.foundationColor;

    Entity road;
    road.id = _nextEntityId++;
    road.type = bp.entityType;
    road.position = Vector3{Float(lx), plan.roadHeight, Float(lz)};
    road.scale = 1.0f;

    int16_t mmDelta = static_cast<int16_t>(plan.deltaY * 1000.0f);
    road.data = ((static_cast<uint8_t>(plan.rampDirX) & 0x0F) << 4) |
                ((static_cast<uint8_t>(plan.rampDirZ) & 0x0F) << 8) |
                ((static_cast<uint32_t>(mmDelta) & 0xFFFF) << 16);

    chunk->addEntity(road);
    if(outDirtyMeshes) {
        outDirtyMeshes->push_back(chunk->buildCpuMesh(world.tileSpacing()));
    }
    RoadNetwork::updateConnectionsAround(world, wx, wz, outDirtyMeshes);
    return true;
}

bool BuildSystem::executeStructurePlacement(World& world, const World::RaycastHit& hit,
                                            const BuildingBlueprint& bp, std::vector<ChunkMeshData>* outDirtyMeshes) {
    Chunk* ownerChunk = world.chunk(hit.chunkCoord);
    if(!ownerChunk) return false;

    Int wx = hit.worldTileX;
    Int wz = hit.worldTileZ;
    Int lx = hit.localTileCoord.x();
    Int lz = hit.localTileCoord.y();

    Vector2i fp = BuildingCatalog::effectiveFootprint(bp.entityType, _buildingOrientation);

    // 1. Compute average foundation elevation across the NxM footprint (even across chunk boundaries)
    Float sumH = 0.0f;
    for(Int dz = 0; dz < fp.y(); ++dz) {
        for(Int dx = 0; dx < fp.x(); ++dx) {
            if(const Tile* ft = world.tileAt(wx + dx, wz + dz)) {
                sumH += ft->height;
            }
        }
    }
    Float avgH = sumH / Float(fp.x() * fp.y());
    Float foundationH = std::round(avgH / 0.2f) * 0.2f;

    std::set<std::pair<Int, Int>> dirtyChunks;
    dirtyChunks.insert({hit.chunkCoord.x(), hit.chunkCoord.y()});

    // 2. Clear any natural flora/boulders and grade/pave all footprint tiles across chunks
    for(Int dz = 0; dz < fp.y(); ++dz) {
        for(Int dx = 0; dx < fp.x(); ++dx) {
            Int tx = wx + dx;
            Int tz = wz + dz;
            Vector2i occOwner;
            if(const Entity* occ = world.entityAt(tx, tz, &occOwner)) {
                if(occ->type == EntityType::TreeOak ||
                   occ->type == EntityType::TreePine ||
                   occ->type == EntityType::Rock) {
                    world.removeEntityAt(tx, tz);
                    dirtyChunks.insert({occOwner.x(), occOwner.y()});
                }
            }
            if(bp.flattenFoundation) {
                if(Tile* ft = world.tileAt(tx, tz)) {
                    ft->height = foundationH;
                    ft->type = TileType::Road;
                    ft->color = bp.foundationColor;
                    Vector2i cc = World::worldToChunkCoord(tx, tz);
                    dirtyChunks.insert({cc.x(), cc.y()});
                }
            }
        }
    }

    // 3. Determine orientation (auto-align to adjacent road across chunks if enabled)
    UnsignedInt orient = _buildingOrientation;
    if(bp.autoAlignToRoad) {
        orient = RoadNetwork::findRoadFacingOrientation(world, wx, wz, fp, _buildingOrientation);
    }

    // 4. Spawn entity in the anchor chunk centered on the NxM footprint
    Entity structure;
    structure.id = _nextEntityId++;
    structure.type = bp.entityType;
    structure.position = Vector3{
        Float(lx) + Float(fp.x() - 1) * 0.5f,
        foundationH,
        Float(lz) + Float(fp.y() - 1) * 0.5f
    };
    structure.scale = 1.0f;
    structure.data = orient & 3u;

    ownerChunk->addEntity(structure);

    // 5. Rebuild CPU meshes of all chunks modified by this structure
    for(const auto& ccPair : dirtyChunks) {
        if(Chunk* ch = world.chunk({ccPair.first, ccPair.second})) {
            if(outDirtyMeshes) {
                outDirtyMeshes->push_back(ch->buildCpuMesh(world.tileSpacing()));
            }
        }
    }
    return true;
}

}
