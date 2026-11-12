#include "world/World.h"
#include "world/RandomWorldGenerator.h"
#include "world/WorldStorage.h"
#include <Corrade/Utility/Debug.h>
#include <cmath>

namespace Magnum {

World::World(Float tileSpacing, std::shared_ptr<WorldGenerator> generator):
    _tileSpacing{tileSpacing},
    _generator{std::move(generator)}
{
    if(!_generator) {
        _generator = std::make_shared<RandomWorldGenerator>();
    }
    WorldStorage::loadOrGenerate(*this, "world.save");
}

void World::setGenerator(std::shared_ptr<WorldGenerator> generator) {
    _generator = std::move(generator);
    init();
}

void World::init(Int chunkWidth, Int chunkHeight) {
    _chunkWidth = chunkWidth;
    _chunkHeight = chunkHeight;
    _chunks.clear();

    const Int minCX = -_chunkWidth / 2;
    const Int maxCX = minCX + _chunkWidth - 1;
    const Int minCZ = -_chunkHeight / 2;
    const Int maxCZ = minCZ + _chunkHeight - 1;

    for(Int cz = minCZ; cz <= maxCZ; ++cz) {
        for(Int cx = minCX; cx <= maxCX; ++cx) {
            Chunk& c = getOrCreateChunk({cx, cz});
            if(_generator) {
                _generator->generateChunk(c);
            }
            c.buildMesh(_tileSpacing);
        }
    }
}

World::RaycastHit World::raycastTile(const Vector3& rayOrigin, const Vector3& rayDir) const {
    RaycastHit bestHit;
    Float closestDistance = 1e9f;

    const Float halfSize = 0.5f;
    const Float chunkWorldSize = chunkSizeInWorldUnits();

    // Raycast through all chunks and tiles (oriented bounding box / cube test)
    for(const auto& entry : _chunks) {
        const Chunk& ch = entry.second;
        Vector3 chunkOrigin{
            Float(ch.chunkCoord().x()) * chunkWorldSize,
            0.0f,
            Float(ch.chunkCoord().y()) * chunkWorldSize
        };

        for(Int z = 0; z < Chunk::Size; ++z) {
            for(Int x = 0; x < Chunk::Size; ++x) {
                const Tile& t = ch.tile(x, z);
                if(!t.active) continue;

                Vector3 minB{
                    chunkOrigin.x() + Float(x) * _tileSpacing - halfSize,
                    0.0f,
                    chunkOrigin.z() + Float(z) * _tileSpacing - halfSize
                };
                Vector3 maxB{
                    chunkOrigin.x() + Float(x) * _tileSpacing + halfSize,
                    t.height,
                    chunkOrigin.z() + Float(z) * _tileSpacing + halfSize
                };

                // Ray vs AABB intersection
                Float tmin = 0.0f;
                Float tmax = 1e9f;

                bool hitBox = true;
                for(int i = 0; i < 3; ++i) {
                    Float invD = 1.0f / rayDir[i];
                    Float t0 = (minB[i] - rayOrigin[i]) * invD;
                    Float t1 = (maxB[i] - rayOrigin[i]) * invD;
                    if(invD < 0.0f) std::swap(t0, t1);

                    tmin = std::max(tmin, t0);
                    tmax = std::min(tmax, t1);

                    if(tmax < tmin) {
                        hitBox = false;
                        break;
                    }
                }

                if(hitBox && tmin < closestDistance) {
                    closestDistance = tmin;
                    bestHit.hit = true;
                    bestHit.chunkCoord = ch.chunkCoord();
                    bestHit.localTileCoord = {x, z};
                    bestHit.worldTileX = ch.chunkCoord().x() * Chunk::Size + x;
                    bestHit.worldTileZ = ch.chunkCoord().y() * Chunk::Size + z;
                    bestHit.worldPoint = rayOrigin + rayDir * tmin;
                }
            }
        }
    }

    return bestHit;
}

Vector2i World::worldToChunkCoord(Int worldX, Int worldZ) {
    Int cx = worldX >= 0 ? (worldX / Chunk::Size) : ((worldX - Chunk::Size + 1) / Chunk::Size);
    Int cz = worldZ >= 0 ? (worldZ / Chunk::Size) : ((worldZ - Chunk::Size + 1) / Chunk::Size);
    return {cx, cz};
}

Vector2i World::worldToLocalTileCoord(Int worldX, Int worldZ) {
    Int lx = (worldX % Chunk::Size + Chunk::Size) % Chunk::Size;
    Int lz = (worldZ % Chunk::Size + Chunk::Size) % Chunk::Size;
    return {lx, lz};
}

Chunk& World::getOrCreateChunk(const Vector2i& chunkCoord) {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    if(it != _chunks.end()) {
        return it->second;
    }
    auto inserted = _chunks.emplace(key, Chunk{chunkCoord});
    return inserted.first->second;
}

Chunk* World::chunk(const Vector2i& chunkCoord) {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    return it != _chunks.end() ? &it->second : nullptr;
}

const Chunk* World::chunk(const Vector2i& chunkCoord) const {
    auto key = std::make_pair(chunkCoord.x(), chunkCoord.y());
    auto it = _chunks.find(key);
    return it != _chunks.end() ? &it->second : nullptr;
}

Tile* World::tileAt(Int worldX, Int worldZ) {
    Vector2i cc = worldToChunkCoord(worldX, worldZ);
    Chunk* ch = chunk(cc);
    if(!ch) return nullptr;
    Vector2i local = worldToLocalTileCoord(worldX, worldZ);
    return &ch->tile(local.x(), local.y());
}

const Tile* World::tileAt(Int worldX, Int worldZ) const {
    Vector2i cc = worldToChunkCoord(worldX, worldZ);
    const Chunk* ch = chunk(cc);
    if(!ch) return nullptr;
    Vector2i local = worldToLocalTileCoord(worldX, worldZ);
    return &ch->tile(local.x(), local.y());
}

Entity* World::entityAt(Int worldX, Int worldZ, Vector2i* outOwnerChunk) {
    Vector2i centerCC = worldToChunkCoord(worldX, worldZ);
    // Search target chunk and immediate neighbor chunks so multi-tile buildings crossing borders are found
    for(Int dz = -1; dz <= 1; ++dz) {
        for(Int dx = -1; dx <= 1; ++dx) {
            Vector2i cc{centerCC.x() + dx, centerCC.y() + dz};
            Chunk* ch = chunk(cc);
            if(!ch) continue;

            Float chunkBaseX = Float(cc.x() * Chunk::Size);
            Float chunkBaseZ = Float(cc.y() * Chunk::Size);
            for(auto& e : ch->entities()) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(e.type, e.data & 3u);
                Float wx = chunkBaseX + e.position.x();
                Float wz = chunkBaseZ + e.position.z();
                if(Math::abs(wx - Float(worldX)) <= halfExt.x() &&
                   Math::abs(wz - Float(worldZ)) <= halfExt.y()) {
                    if(outOwnerChunk) *outOwnerChunk = cc;
                    return &e;
                }
            }
        }
    }
    return nullptr;
}

const Entity* World::entityAt(Int worldX, Int worldZ, Vector2i* outOwnerChunk) const {
    Vector2i centerCC = worldToChunkCoord(worldX, worldZ);
    for(Int dz = -1; dz <= 1; ++dz) {
        for(Int dx = -1; dx <= 1; ++dx) {
            Vector2i cc{centerCC.x() + dx, centerCC.y() + dz};
            const Chunk* ch = chunk(cc);
            if(!ch) continue;

            Float chunkBaseX = Float(cc.x() * Chunk::Size);
            Float chunkBaseZ = Float(cc.y() * Chunk::Size);
            for(const auto& e : ch->entities()) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(e.type, e.data & 3u);
                Float wx = chunkBaseX + e.position.x();
                Float wz = chunkBaseZ + e.position.z();
                if(Math::abs(wx - Float(worldX)) <= halfExt.x() &&
                   Math::abs(wz - Float(worldZ)) <= halfExt.y()) {
                    if(outOwnerChunk) *outOwnerChunk = cc;
                    return &e;
                }
            }
        }
    }
    return nullptr;
}

bool World::removeEntityAt(Int worldX, Int worldZ, Entity* outRemoved, Vector2i* outOwnerChunk) {
    Vector2i centerCC = worldToChunkCoord(worldX, worldZ);
    for(Int dz = -1; dz <= 1; ++dz) {
        for(Int dx = -1; dx <= 1; ++dx) {
            Vector2i cc{centerCC.x() + dx, centerCC.y() + dz};
            Chunk* ch = chunk(cc);
            if(!ch) continue;

            Float chunkBaseX = Float(cc.x() * Chunk::Size);
            Float chunkBaseZ = Float(cc.y() * Chunk::Size);
            auto& ents = ch->entities();
            for(auto it = ents.begin(); it != ents.end(); ++it) {
                Vector2 halfExt = BuildingCatalog::entityHitHalfExtents(it->type, it->data & 3u);
                Float wx = chunkBaseX + it->position.x();
                Float wz = chunkBaseZ + it->position.z();
                if(Math::abs(wx - Float(worldX)) <= halfExt.x() &&
                   Math::abs(wz - Float(worldZ)) <= halfExt.y()) {
                    if(outRemoved) *outRemoved = *it;
                    if(outOwnerChunk) *outOwnerChunk = cc;
                    ents.erase(it);
                    return true;
                }
            }
        }
    }
    return false;
}

}
