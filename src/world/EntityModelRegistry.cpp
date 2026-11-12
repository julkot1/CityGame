#include "world/EntityModelRegistry.h"
#include "world/ModelCache.h"
#include <Magnum/Math/Color.h>
#include <random>
#include <algorithm>

namespace Magnum {

using namespace Math::Literals;

void EntityModelRegistry::buildEntityMesh(const Entity& entity,
                                         std::vector<ChunkVertex>& vertices,
                                         std::vector<UnsignedInt>& indices) {
    switch(entity.type) {
        case EntityType::TreeOak:
            buildOakTree(entity.position, entity.scale, vertices, indices);
            break;
        case EntityType::TreePine:
            buildPineTree(entity.position, entity.scale, vertices, indices);
            break;
        case EntityType::Rock:
            buildRock(entity.position, entity.scale, entity.id, vertices, indices);
            break;
        case EntityType::CoalDeposit:
        case EntityType::IronDeposit:
        case EntityType::CopperDeposit:
            buildOreDeposit(entity.position, entity.scale, entity.id, entity.type, vertices, indices);
            break;
        case EntityType::AncientRuins:
            buildAncientRuins(entity.position, entity.scale, entity.id, vertices, indices);
            break;
        case EntityType::House2x2:
            buildVictorianHouse2x2(entity.position, entity.scale, entity.id, entity.data & 0x03u, vertices, indices);
            break;
        case EntityType::HouseStandalone2x2:
            buildVictorianHouseStandalone2x2(entity.position, entity.scale, entity.id, entity.data & 0x03u, vertices, indices);
            break;
        case EntityType::CityCenter2x3:
            buildCityCenter2x3(entity.position, entity.scale, entity.id, entity.data & 0x03u, vertices, indices);
            break;
        case EntityType::CityPlaza4x2:
            buildCityPlaza4x2(entity.position, entity.scale, entity.id, entity.data & 0x03u, vertices, indices);
            break;
        case EntityType::Warehouse4x4:
            buildWarehouse4x4(entity.position, entity.scale, entity.id, entity.data & 0x03u, vertices, indices);
            break;
        case EntityType::RoadCobblestone: {
            UnsignedByte mask = static_cast<uint8_t>(entity.data & 0x0F);
            int8_t rawDirX = static_cast<int8_t>((entity.data >> 4) & 0x0F);
            if(rawDirX > 7) rawDirX -= 16;
            int8_t rawDirZ = static_cast<int8_t>((entity.data >> 8) & 0x0F);
            if(rawDirZ > 7) rawDirZ -= 16;
            Float deltaY = static_cast<int16_t>((entity.data >> 16) & 0xFFFF) / 1000.0f;

            buildCobblestoneRoad(entity.position, vertices, indices, mask, deltaY, rawDirX, rawDirZ);
            break;
        }
        case EntityType::None:
        default:
            break;
    }
}

void EntityModelRegistry::addAABB(const Vector3& minCorner, const Vector3& maxCorner,
                                 const Color3& color, Float baseAO,
                                 std::vector<ChunkVertex>& vertices,
                                 std::vector<UnsignedInt>& indices) {
    struct Face {
        Vector3 normal;
        Vector3 v0, v1, v2, v3;
    };

    const Vector3& c0 = minCorner;
    const Vector3& c1 = maxCorner;

    Face faces[6] = {
        {{ 0.0f,  1.0f,  0.0f}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
        {{ 0.0f, -1.0f,  0.0f}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}},
        {{ 0.0f,  0.0f,  1.0f}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
        {{ 0.0f,  0.0f, -1.0f}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}},
        {{-1.0f,  0.0f,  0.0f}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c0.z()}},
        {{ 1.0f,  0.0f,  0.0f}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c1.z()}}
    };

    for(const auto& f : faces) {
        UnsignedInt base = vertices.size();
        vertices.push_back({f.v0, f.normal, color, baseAO});
        vertices.push_back({f.v1, f.normal, color, baseAO});
        vertices.push_back({f.v2, f.normal, color, baseAO});
        vertices.push_back({f.v3, f.normal, color, baseAO});

        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 0);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }
}

void EntityModelRegistry::buildOakTree(const Vector3& pos, Float scale,
                                      std::vector<ChunkVertex>& vertices,
                                      std::vector<UnsignedInt>& indices) {
    uint32_t seed = static_cast<uint32_t>(pos.x() * 73856093.0f) ^ static_cast<uint32_t>(pos.z() * 19349663.0f);
    ModelCache::appendModel("assets/models/flora/tree_oak.glb", pos, scale, seed % 4u, seed, vertices, indices);
}

void EntityModelRegistry::buildPineTree(const Vector3& pos, Float scale,
                                       std::vector<ChunkVertex>& vertices,
                                       std::vector<UnsignedInt>& indices) {
    uint32_t seed = static_cast<uint32_t>(pos.x() * 2654435761u) ^ static_cast<uint32_t>(pos.z() * 2246822519u);
    ModelCache::appendModel("assets/models/flora/tree_pine.glb", pos, scale, seed % 4u, 0, vertices, indices);
}

void EntityModelRegistry::buildRock(const Vector3& pos, Float scale, UnsignedInt seed,
                                    std::vector<ChunkVertex>& vertices,
                                    std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/resources/rock.glb", pos, scale, seed % 4u, seed, vertices, indices);
}

void EntityModelRegistry::buildOreDeposit(const Vector3& pos, Float scale, UnsignedInt seed,
                                          EntityType oreType,
                                          std::vector<ChunkVertex>& vertices,
                                          std::vector<UnsignedInt>& indices) {
    const char* modelPath = "assets/models/resources/ore_coal.glb";
    if(oreType == EntityType::IronDeposit) {
        modelPath = "assets/models/resources/ore_iron.glb";
    } else if(oreType == EntityType::CopperDeposit) {
        modelPath = "assets/models/resources/ore_copper.glb";
    }
    ModelCache::appendModel(modelPath, pos, scale, seed % 4u, seed, vertices, indices);
}

void EntityModelRegistry::buildAncientRuins(const Vector3& pos, Float scale, UnsignedInt seed,
                                            std::vector<ChunkVertex>& vertices,
                                            std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/resources/ancient_ruins.glb", pos, scale, seed % 4u, seed, vertices, indices);
}

void EntityModelRegistry::buildVictorianHouse2x2(const Vector3& pos, Float scale,
                                                 UnsignedInt seed, UnsignedInt orientation,
                                                 std::vector<ChunkVertex>& vertices,
                                                 std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/buildings/house_2x2.glb", pos, scale, orientation, seed, vertices, indices);
}

void EntityModelRegistry::buildVictorianHouseStandalone2x2(const Vector3& pos, Float scale,
                                                         UnsignedInt seed, UnsignedInt orientation,
                                                         std::vector<ChunkVertex>& vertices,
                                                         std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/buildings/house_standalone_2x2.glb", pos, scale, orientation, seed, vertices, indices);
}

void EntityModelRegistry::buildCityCenter2x3(const Vector3& pos, Float scale,
                                           UnsignedInt seed, UnsignedInt orientation,
                                           std::vector<ChunkVertex>& vertices,
                                           std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/buildings/city_center_2x3.glb", pos, scale, orientation, seed, vertices, indices);
}

void EntityModelRegistry::buildCityPlaza4x2(const Vector3& pos, Float scale,
                                          UnsignedInt seed, UnsignedInt orientation,
                                          std::vector<ChunkVertex>& vertices,
                                          std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/buildings/city_plaza_4x2.glb", pos, scale, orientation, seed, vertices, indices);
}

void EntityModelRegistry::buildWarehouse4x4(const Vector3& pos, Float scale,
                                            UnsignedInt seed, UnsignedInt orientation,
                                            std::vector<ChunkVertex>& vertices,
                                            std::vector<UnsignedInt>& indices) {
    ModelCache::appendModel("assets/models/buildings/warehouse_4x4.glb", pos, scale, orientation, seed, vertices, indices);
}

void EntityModelRegistry::buildCobblestoneRoad(const Vector3& pos,
                                              std::vector<ChunkVertex>& vertices,
                                              std::vector<UnsignedInt>& indices,
                                              UnsignedByte connectionMask,
                                              Float deltaY,
                                              Int rampDirX,
                                              Int rampDirZ) {
    const Float halfTile = 0.50f;
    const Float roadBedThickness = 0.032f;
    const Color3 sootMortarColor = ModelCache::getMaterial("Mortar_Soot").color;
    const Color3 stoneLight = ModelCache::getMaterial("Cobble_Light").color;
    const Color3 stoneDark = ModelCache::getMaterial("Cobble_Dark").color;
    const Color3 stoneWarm = ModelCache::getMaterial("Cobble_Warm").color;
    const Color3 curbColor = ModelCache::getMaterial("Stone_Curb").color;
    const Float cobbleHeight = 0.024f;

    // If there is no elevation difference, emit flat road according to connectivity mask
    if(Math::abs(deltaY) < 0.01f || (rampDirX == 0 && rampDirZ == 0)) {
        // 1. Soot & mortar roadbed foundation
        addAABB(
            {pos.x() - halfTile, pos.y() + 0.002f, pos.z() - halfTile},
            {pos.x() + halfTile, pos.y() + roadBedThickness, pos.z() + halfTile},
            sootMortarColor, 0.78f, vertices, indices
        );

        // Connections: bit 0 = North (+Z), bit 1 = East (+X), bit 2 = South (-Z), bit 3 = West (-X)
        bool hasN = (connectionMask & 1) != 0;
        bool hasE = (connectionMask & 2) != 0;
        bool hasS = (connectionMask & 4) != 0;
        bool hasW = (connectionMask & 8) != 0;
        int branchCount = (hasN ? 1 : 0) + (hasE ? 1 : 0) + (hasS ? 1 : 0) + (hasW ? 1 : 0);

        // 2. Raised Dressed Granite Curbstones along unconnected street edges
        const Float curbWidth = 0.065f;
        const Float curbTopY = pos.y() + roadBedThickness + cobbleHeight + 0.018f;
        if(!hasN) { // +Z edge curb
            addAABB({pos.x() - halfTile, pos.y() + roadBedThickness, pos.z() + halfTile - curbWidth},
                    {pos.x() + halfTile, curbTopY,                   pos.z() + halfTile},
                    curbColor, 0.92f, vertices, indices);
        }
        if(!hasS) { // -Z edge curb
            addAABB({pos.x() - halfTile, pos.y() + roadBedThickness, pos.z() - halfTile},
                    {pos.x() + halfTile, curbTopY,                   pos.z() - halfTile + curbWidth},
                    curbColor, 0.92f, vertices, indices);
        }
        if(!hasE) { // +X edge curb
            addAABB({pos.x() + halfTile - curbWidth, pos.y() + roadBedThickness, pos.z() - halfTile},
                    {pos.x() + halfTile,             curbTopY,                   pos.z() + halfTile},
                    curbColor, 0.92f, vertices, indices);
        }
        if(!hasW) { // -X edge curb
            addAABB({pos.x() - halfTile,             pos.y() + roadBedThickness, pos.z() - halfTile},
                    {pos.x() - halfTile + curbWidth, curbTopY,                   pos.z() + halfTile},
                    curbColor, 0.92f, vertices, indices);
        }

        // 3. Central cast-iron drain grate / decorative medallion on 3-way or 4-way intersections
        if(branchCount >= 3) {
            const Float plazaHalf = 0.15f;
            addAABB(
                {pos.x() - plazaHalf, pos.y() + roadBedThickness, pos.z() - plazaHalf},
                {pos.x() + plazaHalf, pos.y() + roadBedThickness + cobbleHeight + 0.006f, pos.z() + plazaHalf},
                stoneWarm, 0.98f, vertices, indices
            );
        }

        // 4. Staggered Victorian Cobblestone Setts (4x4 grid with varied stone hues)
        const Float step = 0.22f;
        const Float stoneHalfSize = 0.092f;

        for(int r = 0; r < 4; ++r) {
            for(int c = 0; c < 4; ++c) {
                Float ox = -0.33f + Float(c) * step;
                Float oz = -0.33f + Float(r) * step;
                int pattern = (r * 3 + c * 5 + Int(pos.x() + pos.z())) & 3;
                const Color3& col = (pattern == 0) ? stoneLight : ((pattern == 1) ? stoneWarm : stoneDark);

                addAABB(
                    {pos.x() + ox - stoneHalfSize, pos.y() + roadBedThickness, pos.z() + oz - stoneHalfSize},
                    {pos.x() + ox + stoneHalfSize, pos.y() + roadBedThickness + cobbleHeight, pos.z() + oz + stoneHalfSize},
                    col, 0.94f, vertices, indices
                );
            }
        }

        // 5. Place a Victorian Cast-Iron Gas Streetlamp using external GLB model
        Int tileHash = std::abs(Int(std::round(pos.x())) * 3 + Int(std::round(pos.z())) * 7);
        if(branchCount >= 3 || (branchCount >= 1 && (tileHash % 3 == 0))) {
            Float lampX = pos.x() + (hasE ? -0.40f : 0.40f);
            Float lampZ = pos.z() + (hasN ? -0.40f : 0.40f);
            ModelCache::appendModel("assets/models/infrastructure/gas_lamp.glb",
                                    Vector3{lampX, curbTopY, lampZ},
                                    1.0f, 0, 0, vertices, indices);
        }
        return;
    }

    // SLOPED RAMP ROAD:
    auto evalHeight = [&](Float u) -> Float {
        return pos.y() + 0.002f + u * deltaY;
    };

    Float h00, h10, h11, h01;
    if(rampDirX != 0) {
        Float uLeft  = (rampDirX > 0) ? 0.0f : 1.0f;
        Float uRight = (rampDirX > 0) ? 1.0f : 0.0f;
        h00 = evalHeight(uLeft);
        h01 = evalHeight(uLeft);
        h10 = evalHeight(uRight);
        h11 = evalHeight(uRight);
    } else {
        Float uBack  = (rampDirZ > 0) ? 0.0f : 1.0f;
        Float uFront = (rampDirZ > 0) ? 1.0f : 0.0f;
        h00 = evalHeight(uBack);
        h10 = evalHeight(uBack);
        h01 = evalHeight(uFront);
        h11 = evalHeight(uFront);
    }

    Vector3 topNormal = (rampDirX != 0)
        ? Vector3{-Float(rampDirX) * deltaY, 1.0f, 0.0f}.normalized()
        : Vector3{0.0f, 1.0f, -Float(rampDirZ) * deltaY}.normalized();

    // 1. Emit Sloped Soot/Mortar Roadbed Top
    UnsignedInt base = vertices.size();
    vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, topNormal, sootMortarColor, 0.85f});
    vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, topNormal, sootMortarColor, 0.85f});
    vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, topNormal, sootMortarColor, 0.85f});
    vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, topNormal, sootMortarColor, 0.85f});

    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 1);
    indices.push_back(base + 0);
    indices.push_back(base + 3);
    indices.push_back(base + 2);

    // Lateral masonry retaining walls for the ramp
    if(rampDirX != 0) {
        Float yLow = pos.y() + 0.002f;
        base = vertices.size();
        Vector3 bNorm{0.0f, 0.0f, -1.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, bNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() - halfTile}, bNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, bNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, bNorm, curbColor, 0.88f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        base = vertices.size();
        Vector3 fNorm{0.0f, 0.0f, 1.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() + halfTile}, fNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, fNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, fNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() + halfTile}, fNorm, curbColor, 0.70f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
    } else {
        Float yLow = pos.y() + 0.002f;
        base = vertices.size();
        Vector3 lNorm{-1.0f, 0.0f, 0.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, lNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, lNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, lNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() + halfTile}, lNorm, curbColor, 0.70f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        base = vertices.size();
        Vector3 rNorm{1.0f, 0.0f, 0.0f};
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() - halfTile}, rNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() + halfTile}, rNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, rNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, rNorm, curbColor, 0.88f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
    }

    // 2. Emit 4 Distinct Stepped Paver Tiers climbing along the incline
    const Float step = 0.22f;
    const Float stoneHalfSize = 0.092f;
    const Float stepElevationInterval = deltaY / 4.0f;

    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c) {
            Float ox = -0.33f + Float(c) * step;
            Float oz = -0.33f + Float(r) * step;

            Int stepIndex = 0;
            if(rampDirX != 0) {
                stepIndex = (rampDirX > 0) ? c : (3 - c);
            } else {
                stepIndex = (rampDirZ > 0) ? r : (3 - r);
            }

            Float tierY = pos.y() + 0.002f + Float(stepIndex) * stepElevationInterval + roadBedThickness;
            int pattern = (r * 3 + c * 5) & 3;
            const Color3& col = (pattern == 0) ? stoneLight : ((pattern == 1) ? stoneWarm : stoneDark);

            addAABB(
                {pos.x() + ox - stoneHalfSize, tierY, pos.z() + oz - stoneHalfSize},
                {pos.x() + ox + stoneHalfSize, tierY + cobbleHeight + stepElevationInterval * 0.4f, pos.z() + oz + stoneHalfSize},
                col, 0.94f, vertices, indices
            );
        }
    }
}

}
