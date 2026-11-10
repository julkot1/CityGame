#include "world/WorldStorage.h"
#include "world/World.h"
#include "world/Entity.h"
#include <Corrade/Utility/Debug.h>
#include <fstream>

namespace Magnum {

namespace {
    constexpr char SaveMagic[4] = {'C', 'I', 'T', 'Y'};
    constexpr UnsignedInt SaveVersion = 2; // Incremented for generic Entity support

    struct SerializedTile {
        UnsignedByte type;
        UnsignedByte active;
        Float height;
        Float r, g, b;
    };

    struct SerializedEntity {
        UnsignedInt id;
        UnsignedShort type;
        Float posX, posY, posZ;
        Float rotationY;
        Float scale;
        UnsignedInt data;
    };
}

bool WorldStorage::saveToFile(const World& world, const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if(!out.is_open()) {
        Debug{} << "Failed to open save file for writing:" << filepath.c_str();
        return false;
    }

    out.write(SaveMagic, 4);
    out.write(reinterpret_cast<const char*>(&SaveVersion), sizeof(SaveVersion));

    Int width = World::WorldChunkWidth;
    Int height = World::WorldChunkHeight;
    Float tileSpacing = world.tileSpacing();

    out.write(reinterpret_cast<const char*>(&width), sizeof(width));
    out.write(reinterpret_cast<const char*>(&height), sizeof(height));
    out.write(reinterpret_cast<const char*>(&tileSpacing), sizeof(tileSpacing));

    UnsignedInt chunkCount = world.chunks().size();
    out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

    for(const auto& entry : world.chunks()) {
        Int cx = entry.first.first;
        Int cz = entry.first.second;
        out.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
        out.write(reinterpret_cast<const char*>(&cz), sizeof(cz));

        const Chunk& chunk = entry.second;
        // 1. Write tiles
        for(Int z = 0; z < Chunk::Size; ++z) {
            for(Int x = 0; x < Chunk::Size; ++x) {
                const Tile& t = chunk.tile(x, z);
                SerializedTile st{
                    static_cast<UnsignedByte>(t.type),
                    static_cast<UnsignedByte>(t.active ? 1 : 0),
                    t.height,
                    t.color.r(),
                    t.color.g(),
                    t.color.b()
                };
                out.write(reinterpret_cast<const char*>(&st), sizeof(st));
            }
        }

        // 2. Write entities
        UnsignedInt entityCount = chunk.entities().size();
        out.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));
        for(const Entity& e : chunk.entities()) {
            SerializedEntity se{
                e.id,
                static_cast<UnsignedShort>(e.type),
                e.position.x(), e.position.y(), e.position.z(),
                e.rotationY,
                e.scale,
                e.data
            };
            out.write(reinterpret_cast<const char*>(&se), sizeof(se));
        }
    }

    Debug{} << "World saved successfully to:" << filepath.c_str() << "(" << chunkCount << "chunks)";
    return true;
}

bool WorldStorage::loadFromFile(World& world, const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if(!in.is_open()) {
        return false;
    }

    char magic[4];
    in.read(magic, 4);
    if(magic[0] != 'C' || magic[1] != 'I' || magic[2] != 'T' || magic[3] != 'Y') {
        Debug{} << "Invalid world save header in:" << filepath.c_str();
        return false;
    }

    UnsignedInt version = 0;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if(version != 1 && version != 2) {
        Debug{} << "Unsupported save file version:" << version;
        return false;
    }

    Int width = 0, height = 0;
    Float tileSpacing = 0.0f;
    in.read(reinterpret_cast<char*>(&width), sizeof(width));
    in.read(reinterpret_cast<char*>(&height), sizeof(height));
    in.read(reinterpret_cast<char*>(&tileSpacing), sizeof(tileSpacing));

    UnsignedInt chunkCount = 0;
    in.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));

    world.clear();

    for(UnsignedInt i = 0; i < chunkCount; ++i) {
        Int cx = 0, cz = 0;
        in.read(reinterpret_cast<char*>(&cx), sizeof(cx));
        in.read(reinterpret_cast<char*>(&cz), sizeof(cz));

        Chunk& chunk = world.getOrCreateChunk({cx, cz});
        chunk.clearEntities();

        // Read tiles
        for(Int z = 0; z < Chunk::Size; ++z) {
            for(Int x = 0; x < Chunk::Size; ++x) {
                SerializedTile st;
                in.read(reinterpret_cast<char*>(&st), sizeof(st));

                Tile& t = chunk.tile(x, z);
                t.type = static_cast<TileType>(st.type);
                t.active = (st.active != 0);
                t.height = st.height;
                t.color = Color3{st.r, st.g, st.b};
            }
        }

        // Read entities (version 2+)
        if(version >= 2) {
            UnsignedInt entityCount = 0;
            in.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));
            for(UnsignedInt j = 0; j < entityCount; ++j) {
                SerializedEntity se;
                in.read(reinterpret_cast<char*>(&se), sizeof(se));
                Entity e;
                e.id = se.id;
                e.type = static_cast<EntityType>(se.type);
                e.position = Vector3{se.posX, se.posY, se.posZ};
                e.rotationY = se.rotationY;
                e.scale = se.scale;
                e.data = se.data;
                chunk.addEntity(e);
            }
        }

        chunk.buildMesh(world.tileSpacing());
    }

    Debug{} << "World loaded successfully from:" << filepath.c_str() << "(" << chunkCount << "chunks)";
    return true;
}

void WorldStorage::loadOrGenerate(World& world, const std::string& filepath) {
    if(loadFromFile(world, filepath)) {
        return;
    }

    Debug{} << "Save file" << filepath.c_str() << "not found. Generating new procedural world...";
    world.init();
    saveToFile(world, filepath);
}

}
