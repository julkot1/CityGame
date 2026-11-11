#include "world/BuildSystem.h"
#include "world/Chunk.h"
#include <Corrade/Utility/Debug.h>

namespace Magnum {

BuildSystem::BuildSystem() = default;

const char* BuildSystem::toolName(BuildTool tool) const {
    switch(tool) {
        case BuildTool::Inspect: return "Inspect & Survey";
        case BuildTool::RoadCobblestone: return "Cobblestone Road";
        case BuildTool::Demolish: return "Demolish / Bulldoze";
    }
    return "Unknown";
}

PlacementValidation BuildSystem::validatePlacement(const World& world, const World::RaycastHit& hit) const {
    if(!hit.hit) {
        return {false, "No tile selected"};
    }

    const Chunk* chunk = world.chunk(hit.chunkCoord);
    if(!chunk) {
        return {false, "Invalid chunk location"};
    }

    const Tile& tile = chunk->tile(hit.localTileCoord.x(), hit.localTileCoord.y());
    const Entity* existingEntity = chunk->entityAt(hit.localTileCoord.x(), hit.localTileCoord.y());

    if(_activeTool == BuildTool::Demolish) {
        if(!existingEntity) {
            return {false, "No entity on this tile to demolish"};
        }
        return {true, "Ready to bulldoze"};
    }

    if(_activeTool == BuildTool::RoadCobblestone) {
        // Cannot build roads on open water
        if(tile.type == TileType::Water) {
            return {false, "Cannot build road on water"};
        }
        // Cannot place over existing trees/rocks/buildings without bulldozing first
        if(existingEntity) {
            if(existingEntity->type == EntityType::RoadCobblestone) {
                return {false, "Road already exists here"};
            }
            return {false, "Tile obstructed (clear area first)"};
        }

        // Check if there is an adjacent road to connect with
        Int lx = hit.localTileCoord.x();
        Int lz = hit.localTileCoord.y();
        const Int neighbors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for(const auto& n : neighbors) {
            Int nx = lx + n[0];
            Int nz = lz + n[1];
            if(nx >= 0 && nx < Chunk::Size && nz >= 0 && nz < Chunk::Size) {
                const Entity* ne = chunk->entityAt(nx, nz);
                if(ne && ne->type == EntityType::RoadCobblestone) {
                    Float diff = Math::abs(ne->position.y() - tile.height);
                    if(diff > 0.65f) {
                        return {false, "Cliff too steep to grade road (max 0.6m)"};
                    }
                }
            }
        }

        return {true, "Ready to construct road"};
    }

    return {true, "OK"};
}

UnsignedByte BuildSystem::computeRoadMask(const World& world, const Vector2i& chunkCoord, Int localX, Int localZ) {
    const Chunk* chunk = world.chunk(chunkCoord);
    if(!chunk) return 0;

    UnsignedByte mask = 0;
    // N = +Z (1), E = +X (2), S = -Z (4), W = -X (8)
    const struct { Int dx; Int dz; UnsignedByte bit; } neighbors[] = {
        { 0,  1, 1}, // North (+Z)
        { 1,  0, 2}, // East  (+X)
        { 0, -1, 4}, // South (-Z)
        {-1,  0, 8}  // West  (-X)
    };

    for(const auto& n : neighbors) {
        Int nx = localX + n.dx;
        Int nz = localZ + n.dz;
        if(nx >= 0 && nx < Chunk::Size && nz >= 0 && nz < Chunk::Size) {
            const Entity* ne = chunk->entityAt(nx, nz);
            if(ne && ne->type == EntityType::RoadCobblestone) {
                mask |= n.bit;
            }
        }
    }

    return mask;
}

void BuildSystem::updateRoadConnectionsAround(World& world, const Vector2i& chunkCoord, Int localX, Int localZ) {
    Chunk* chunk = world.chunk(chunkCoord);
    if(!chunk) return;

    // Self + 4 neighbors
    const Int offsets[5][2] = {{0, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    for(const auto& off : offsets) {
        Int nx = localX + off[0];
        Int nz = localZ + off[1];
        if(nx >= 0 && nx < Chunk::Size && nz >= 0 && nz < Chunk::Size) {
            Entity* e = chunk->entityAt(nx, nz);
            if(e && e->type == EntityType::RoadCobblestone) {
                UnsignedByte mask = computeRoadMask(world, chunkCoord, nx, nz);
                // Preserve upper ramp bits, update lower 4 bits (mask)
                e->data = (e->data & ~0x0Fu) | (mask & 0x0Fu);
            }
        }
    }
}

bool BuildSystem::executeAction(World& world, const World::RaycastHit& hit) {
    PlacementValidation val = validatePlacement(world, hit);
    if(!val.valid) {
        return false;
    }

    Chunk* chunk = world.chunk(hit.chunkCoord);
    if(!chunk) return false;

    Int lx = hit.localTileCoord.x();
    Int lz = hit.localTileCoord.y();

    if(_activeTool == BuildTool::Demolish) {
        if(chunk->removeEntityAt(lx, lz)) {
            // Restore terrain tile color / type if it was a road
            Tile& t = chunk->tile(lx, lz);
            if(t.type == TileType::Road) {
                t.type = TileType::Grass;
                t.color = Color3{0x689d6a_rgbf};
            }
            // Update surrounding roads so they close connection
            updateRoadConnectionsAround(world, hit.chunkCoord, lx, lz);
            chunk->buildMesh(world.tileSpacing());
            return true;
        }
        return false;
    }

    if(_activeTool == BuildTool::RoadCobblestone) {
        Tile& t = chunk->tile(lx, lz);

        // HYBRID TERRAIN LEVELING + STEPPED RAMP:
        // 1. Level small irregularities (diff <= 0.10m) so flat roads remain completely uniform.
        // 2. If diff is between 0.10m and 0.65m, create a stepped cobblestone ramp climbing up to the neighbor!
        Float roadHeight = t.height;
        Int rampDirX = 0;
        Int rampDirZ = 0;
        Float deltaY = 0.0f;

        const Int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for(const auto& d : dirs) {
            Int nx = lx + d[0];
            Int nz = lz + d[1];
            if(nx >= 0 && nx < Chunk::Size && nz >= 0 && nz < Chunk::Size) {
                const Entity* ne = chunk->entityAt(nx, nz);
                if(ne && ne->type == EntityType::RoadCobblestone) {
                    Float diff = ne->position.y() - t.height;
                    if(Math::abs(diff) <= 0.10f) {
                        // Tiny irregularity -> snap/level directly
                        roadHeight = ne->position.y();
                        break;
                    } else if(diff > 0.10f && diff <= 0.65f) {
                        // Neighbor is a plateau step higher -> build stepped ramp upward
                        rampDirX = d[0];
                        rampDirZ = d[1];
                        deltaY = diff;
                        break;
                    }
                }
            }
        }

        // Apply base terrain height and road styling
        t.height = roadHeight;
        t.type = TileType::Road;
        t.color = Color3{0.30f, 0.22f, 0.15f}; // Dark earth foundation

        // Place road entity with step metadata
        Entity road;
        road.id = _nextEntityId++;
        road.type = EntityType::RoadCobblestone;
        road.position = Vector3{Float(lx), roadHeight, Float(lz)};
        road.scale = 1.0f;

        int16_t mmDelta = static_cast<int16_t>(deltaY * 1000.0f);
        road.data = ((static_cast<uint8_t>(rampDirX) & 0x0F) << 4) |
                    ((static_cast<uint8_t>(rampDirZ) & 0x0F) << 8) |
                    ((static_cast<uint32_t>(mmDelta) & 0xFFFF) << 16);

        chunk->addEntity(road);

        // Update auto-tiling connection mask for this road and all adjacent roads
        updateRoadConnectionsAround(world, hit.chunkCoord, lx, lz);

        chunk->buildMesh(world.tileSpacing());
        return true;
    }

    return false;
}

}
