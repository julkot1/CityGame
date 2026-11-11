#include "world/EntityModelRegistry.h"
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
        case EntityType::RoadCobblestone: {
            // Decode road metadata from entity.data:
            // bits 0-3:  connectionMask (N=1, E=2, S=4, W=8)
            // bits 4-7:  rampDirX as signed int4 (-1, 0, 1)
            // bits 8-11: rampDirZ as signed int4 (-1, 0, 1)
            // bits 16-31: deltaY in millimeters
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
    // 6 faces of an axis-aligned bounding box
    struct Face {
        Vector3 normal;
        Vector3 v0, v1, v2, v3;
    };

    const Vector3& c0 = minCorner;
    const Vector3& c1 = maxCorner;

    Face faces[6] = {
        // Top (+Y)
        {{ 0.0f,  1.0f,  0.0f}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
        // Bottom (-Y)
        {{ 0.0f, -1.0f,  0.0f}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}},
        // Front (+Z)
        {{ 0.0f,  0.0f,  1.0f}, {c0.x(), c0.y(), c1.z()}, {c1.x(), c0.y(), c1.z()}, {c1.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}},
        // Back (-Z)
        {{ 0.0f,  0.0f, -1.0f}, {c1.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c1.y(), c0.z()}, {c1.x(), c1.y(), c0.z()}},
        // Left (-X)
        {{-1.0f,  0.0f,  0.0f}, {c0.x(), c0.y(), c0.z()}, {c0.x(), c0.y(), c1.z()}, {c0.x(), c1.y(), c1.z()}, {c0.x(), c1.y(), c0.z()}},
        // Right (+X)
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
    // Deterministic per-tree seed for autumnal/Victorian foliage palette & asymmetrical canopy clusters
    uint32_t hSeed = static_cast<uint32_t>(pos.x() * 73856093.0f) ^ static_cast<uint32_t>(pos.z() * 19349663.0f);
    std::mt19937 rng(hSeed);
    std::uniform_real_distribution<Float> dist01(0.0f, 1.0f);

    // 4 Victorian / Steampunk foliage palettes:
    // 0: Deep mossy olive, 1: Autumnal russet-copper, 2: Golden ochre-bronze, 3: Dark highland sage
    Float paletteRoll = dist01(rng);
    Color3 leafBase, leafMid, leafCrown;
    if(paletteRoll < 0.42f) {
        // Mossy Victorian Olive
        leafBase  = Color3{0.22f, 0.30f, 0.16f};
        leafMid   = Color3{0.28f, 0.37f, 0.19f};
        leafCrown = Color3{0.35f, 0.43f, 0.22f};
    } else if(paletteRoll < 0.70f) {
        // Autumnal Russet & Copper
        leafBase  = Color3{0.38f, 0.20f, 0.11f};
        leafMid   = Color3{0.48f, 0.26f, 0.13f};
        leafCrown = Color3{0.58f, 0.33f, 0.15f};
    } else if(paletteRoll < 0.88f) {
        // Warm Golden Ochre
        leafBase  = Color3{0.42f, 0.32f, 0.14f};
        leafMid   = Color3{0.52f, 0.40f, 0.16f};
        leafCrown = Color3{0.62f, 0.49f, 0.20f};
    } else {
        // Highland Bronze-Green
        leafBase  = Color3{0.19f, 0.27f, 0.18f};
        leafMid   = Color3{0.25f, 0.33f, 0.21f};
        leafCrown = Color3{0.32f, 0.40f, 0.24f};
    }

    const Float trunkRadius = 0.11f * scale;
    const Float trunkHeight = 1.15f * scale;
    const Color3 trunkColor{0.25f, 0.17f, 0.12f}; // Dark weathered oak timber
    const Color3 rootColor{0.21f, 0.14f, 0.10f};

    // 1. Flared Root Base (gives grounded miniature silhouette)
    addAABB(
        {pos.x() - trunkRadius * 1.45f, pos.y(), pos.z() - trunkRadius * 1.45f},
        {pos.x() + trunkRadius * 1.45f, pos.y() + 0.18f * scale, pos.z() + trunkRadius * 1.45f},
        rootColor, 0.60f, vertices, indices
    );

    // 2. Main Trunk
    addAABB(
        {pos.x() - trunkRadius, pos.y(), pos.z() - trunkRadius},
        {pos.x() + trunkRadius, pos.y() + trunkHeight, pos.z() + trunkRadius},
        trunkColor, 0.70f, vertices, indices
    );

    // 3. Exposed Lower Bough / Branch Stub
    Float branchDir = (dist01(rng) > 0.5f) ? 1.0f : -1.0f;
    addAABB(
        {pos.x() + branchDir * trunkRadius * 0.5f, pos.y() + trunkHeight * 0.52f, pos.z() - trunkRadius * 0.65f},
        {pos.x() + branchDir * trunkRadius * 2.6f, pos.y() + trunkHeight * 0.68f, pos.z() + trunkRadius * 0.65f},
        trunkColor, 0.72f, vertices, indices
    );

    // 4. Multi-layered Micro-Voxel Canopy (5 overlapping clusters instead of 2 boxy cubes)
    //    Encoded with +3.0f AO offset so the vertex shader animates wind-swaying foliage!
    const Float baseCanopyY = pos.y() + trunkHeight * 0.68f;
    const Float windTag = 3.0f;

    // Lower wide core
    addAABB(
        {pos.x() - 0.56f * scale, baseCanopyY, pos.z() - 0.46f * scale},
        {pos.x() + 0.56f * scale, baseCanopyY + 0.42f * scale, pos.z() + 0.46f * scale},
        leafBase, windTag + 0.78f, vertices, indices
    );
    addAABB(
        {pos.x() - 0.46f * scale, baseCanopyY + 0.04f * scale, pos.z() - 0.56f * scale},
        {pos.x() + 0.46f * scale, baseCanopyY + 0.44f * scale, pos.z() + 0.56f * scale},
        leafBase, windTag + 0.80f, vertices, indices
    );

    // Offset secondary bough cluster
    Float sideX = (dist01(rng) - 0.5f) * 0.26f * scale;
    Float sideZ = (dist01(rng) - 0.5f) * 0.26f * scale;
    addAABB(
        {pos.x() + sideX - 0.48f * scale, baseCanopyY + 0.28f * scale, pos.z() + sideZ - 0.48f * scale},
        {pos.x() + sideX + 0.48f * scale, baseCanopyY + 0.76f * scale, pos.z() + sideZ + 0.48f * scale},
        leafMid, windTag + 0.88f, vertices, indices
    );

    // Upper-mid canopy dome
    addAABB(
        {pos.x() - 0.40f * scale, baseCanopyY + 0.52f * scale, pos.z() - 0.40f * scale},
        {pos.x() + 0.40f * scale, baseCanopyY + 0.96f * scale, pos.z() + 0.40f * scale},
        leafMid, windTag + 0.92f, vertices, indices
    );

    // Crown top tuft
    addAABB(
        {pos.x() - 0.25f * scale, baseCanopyY + 0.88f * scale, pos.z() - 0.25f * scale},
        {pos.x() + 0.25f * scale, baseCanopyY + 1.16f * scale, pos.z() + 0.25f * scale},
        leafCrown, windTag + 0.98f, vertices, indices
    );
}

void EntityModelRegistry::buildPineTree(const Vector3& pos, Float scale,
                                       std::vector<ChunkVertex>& vertices,
                                       std::vector<UnsignedInt>& indices) {
    const Float trunkRadius = 0.095f * scale;
    const Float trunkHeight = 1.65f * scale;
    const Color3 trunkColor{0.23f, 0.15f, 0.10f}; // Dark pitch-pine timber

    // Deep Victorian conifer & larch needles (muted slate-spruce tones)
    const Color3 pineTier1{0.11f, 0.21f, 0.16f};
    const Color3 pineTier2{0.13f, 0.25f, 0.18f};
    const Color3 pineTier3{0.16f, 0.29f, 0.20f};
    const Color3 pineTier4{0.19f, 0.34f, 0.22f};
    const Color3 pineSpire{0.22f, 0.38f, 0.24f};

    // 1. Flared Root Base
    addAABB(
        {pos.x() - trunkRadius * 1.4f, pos.y(), pos.z() - trunkRadius * 1.4f},
        {pos.x() + trunkRadius * 1.4f, pos.y() + 0.16f * scale, pos.z() + trunkRadius * 1.4f},
        trunkColor * 0.85f, 0.60f, vertices, indices
    );

    // 2. Slender Trunk
    addAABB(
        {pos.x() - trunkRadius, pos.y(), pos.z() - trunkRadius},
        {pos.x() + trunkRadius, pos.y() + trunkHeight, pos.z() + trunkRadius},
        trunkColor, 0.70f, vertices, indices
    );

    // Helper to build a cross-shaped (octagonal-like) conifer tier with +3.0f wind animation tag
    const Float windTag = 3.0f;
    auto addConiferTier = [&](Float y0, Float y1, Float rOuter, Float rInner, const Color3& col, Float ao) {
        addAABB(
            {pos.x() - rOuter, y0, pos.z() - rInner},
            {pos.x() + rOuter, y1, pos.z() + rInner},
            col, windTag + ao, vertices, indices
        );
        addAABB(
            {pos.x() - rInner, y0, pos.z() - rOuter},
            {pos.x() + rInner, y1, pos.z() + rOuter},
            col, windTag + ao, vertices, indices
        );
    };

    // 5 finely stepped conifer tiers up to a sharp spire
    Float yBase = pos.y() + trunkHeight * 0.36f;
    addConiferTier(yBase,                  yBase + 0.38f * scale, 0.62f * scale, 0.44f * scale, pineTier1, 0.76f);
    addConiferTier(yBase + 0.32f * scale,  yBase + 0.68f * scale, 0.50f * scale, 0.35f * scale, pineTier2, 0.82f);
    addConiferTier(yBase + 0.62f * scale,  yBase + 0.96f * scale, 0.38f * scale, 0.27f * scale, pineTier3, 0.88f);
    addConiferTier(yBase + 0.90f * scale,  yBase + 1.22f * scale, 0.26f * scale, 0.18f * scale, pineTier4, 0.93f);

    // Pointed top spire
    addAABB(
        {pos.x() - 0.12f * scale, yBase + 1.18f * scale, pos.z() - 0.12f * scale},
        {pos.x() + 0.12f * scale, yBase + 1.50f * scale, pos.z() + 0.12f * scale},
        pineSpire, windTag + 0.98f, vertices, indices
    );
}


void EntityModelRegistry::buildRock(const Vector3& pos, Float scale, UnsignedInt seed,
                                    std::vector<ChunkVertex>& vertices,
                                    std::vector<UnsignedInt>& indices) {
    // Deterministic PRNG seeded with entity ID and position so rock shape stays constant forever
    uint32_t s = seed ^ static_cast<uint32_t>(pos.x() * 1000.0f) ^ (static_cast<uint32_t>(pos.z() * 1000.0f) << 16);
    std::mt19937 gen(s);

    std::uniform_real_distribution<Float> sizeDist(0.70f, 0.95f);
    std::uniform_real_distribution<Float> colorDist(-0.035f, 0.035f);
    std::uniform_real_distribution<Float> offsetDist(-0.08f, 0.08f);

    // Dark Victorian basalt & slate rock tone
    const Color3 baseColor{0.34f, 0.35f, 0.38f};

    // Constant rock dimensions for this rock instance
    const Float width = scale * sizeDist(gen);
    const Float height = scale * sizeDist(gen) * 0.65f;
    const Float depth = scale * sizeDist(gen);

    const Float ox = offsetDist(gen) * scale;
    const Float oz = offsetDist(gen) * scale;

    auto randomColor = [&]() {
        Float variation = colorDist(gen);

        return Color3{
            std::clamp(baseColor.r() + variation, 0.0f, 1.0f),
            std::clamp(baseColor.g() + variation, 0.0f, 1.0f),
            std::clamp(baseColor.b() + variation * 1.1f, 0.0f, 1.0f)
        };
    };

    // 1. Wide base slate slab
    addAABB(
        {pos.x() - width * 0.56f + ox, pos.y(), pos.z() - depth * 0.52f + oz},
        {pos.x() + width * 0.56f + ox, pos.y() + height * 0.42f, pos.z() + depth * 0.52f + oz},
        randomColor() * 0.90f, 0.65f, vertices, indices
    );

    // 2. Main angular body
    addAABB(
        {pos.x() - width * 0.48f + ox, pos.y(), pos.z() - depth * 0.48f + oz},
        {pos.x() + width * 0.48f + ox, pos.y() + height, pos.z() + depth * 0.48f + oz},
        randomColor(), 0.76f, vertices, indices
    );

    // 3. Upper stepped crag
    const Float topWidth = width * 0.62f;
    const Float topDepth = depth * 0.66f;
    addAABB(
        {pos.x() - topWidth * 0.5f + ox, pos.y() + height * 0.65f, pos.z() - topDepth * 0.5f + oz},
        {pos.x() + topWidth * 0.5f + ox, pos.y() + height * 1.18f, pos.z() + topDepth * 0.5f + oz},
        randomColor(), 0.86f, vertices, indices
    );

    // 4. Mossy highland lichen cap on top of the crag
    const Color3 mossColor{0.31f, 0.39f, 0.24f};
    addAABB(
        {pos.x() - topWidth * 0.44f + ox, pos.y() + height * 1.18f, pos.z() - topDepth * 0.44f + oz},
        {pos.x() + topWidth * 0.44f + ox, pos.y() + height * 1.24f, pos.z() + topDepth * 0.44f + oz},
        mossColor, 0.94f, vertices, indices
    );

    // 5. Lateral buttress slab
    addAABB(
        {pos.x() - width * 0.60f + ox, pos.y() + height * 0.12f, pos.z() - depth * 0.28f + oz},
        {pos.x() - width * 0.20f + ox, pos.y() + height * 0.68f, pos.z() + depth * 0.28f + oz},
        randomColor(), 0.68f, vertices, indices
    );
}

void EntityModelRegistry::buildOreDeposit(const Vector3& pos, Float scale, UnsignedInt seed,
                                          EntityType oreType,
                                          std::vector<ChunkVertex>& vertices,
                                          std::vector<UnsignedInt>& indices) {
    uint32_t s = seed ^ static_cast<uint32_t>(pos.x() * 1000.0f) ^ (static_cast<uint32_t>(pos.z() * 1000.0f) << 16);
    std::mt19937 gen(s);
    std::uniform_real_distribution<Float> sizeDist(0.75f, 0.95f);

    const Float w = scale * sizeDist(gen);
    const Float h = scale * sizeDist(gen) * 0.72f;
    const Float d = scale * sizeDist(gen);

    // Host rock matrix color + Mineral vein colors
    Color3 matrixRock{0.31f, 0.32f, 0.35f};
    Color3 veinPrimary, veinSecondary, veinAccent;

    if(oreType == EntityType::CoalDeposit) {
        // Anthracite Black Coal Seam
        matrixRock    = Color3{0.24f, 0.25f, 0.27f};
        veinPrimary   = Color3{0.10f, 0.11f, 0.13f};
        veinSecondary = Color3{0.15f, 0.16f, 0.18f};
        veinAccent    = Color3{0.07f, 0.08f, 0.10f};
    } else if(oreType == EntityType::IronDeposit) {
        // Rust-Ochre & Hematite Iron Ore Vein
        matrixRock    = Color3{0.34f, 0.30f, 0.28f};
        veinPrimary   = Color3{0.52f, 0.24f, 0.16f};
        veinSecondary = Color3{0.64f, 0.31f, 0.19f};
        veinAccent    = Color3{0.38f, 0.20f, 0.15f};
    } else {
        // Verdigris Malachite & Tarnished Bronze Copper Vein
        matrixRock    = Color3{0.32f, 0.34f, 0.33f};
        veinPrimary   = Color3{0.24f, 0.48f, 0.40f};
        veinSecondary = Color3{0.58f, 0.36f, 0.20f};
        veinAccent    = Color3{0.30f, 0.58f, 0.47f};
    }

    // 1. Base host rock bed
    addAABB(
        {pos.x() - w * 0.55f, pos.y(),          pos.z() - d * 0.55f},
        {pos.x() + w * 0.55f, pos.y() + h * 0.45f, pos.z() + d * 0.55f},
        matrixRock, 0.68f, vertices, indices
    );

    // 2. Main mineral seam block
    addAABB(
        {pos.x() - w * 0.44f, pos.y() + h * 0.20f, pos.z() - d * 0.44f},
        {pos.x() + w * 0.44f, pos.y() + h * 0.95f, pos.z() + d * 0.44f},
        veinPrimary, 0.80f, vertices, indices
    );

    // 3. Protruding diagonal/stepped ore crystals / strata slabs
    addAABB(
        {pos.x() - w * 0.52f, pos.y() + h * 0.35f, pos.z() - d * 0.22f},
        {pos.x() + w * 0.18f, pos.y() + h * 1.12f, pos.z() + d * 0.38f},
        veinSecondary, 0.88f, vertices, indices
    );

    addAABB(
        {pos.x() + w * 0.05f, pos.y() + h * 0.48f, pos.z() - d * 0.48f},
        {pos.x() + w * 0.48f, pos.y() + h * 1.22f, pos.z() + d * 0.12f},
        veinAccent, 0.94f, vertices, indices
    );
}

void EntityModelRegistry::buildAncientRuins(const Vector3& pos, Float scale, UnsignedInt seed,
                                            std::vector<ChunkVertex>& vertices,
                                            std::vector<UnsignedInt>& indices) {
    uint32_t s = seed ^ static_cast<uint32_t>(pos.x() * 1000.0f);
    std::mt19937 gen(s);
    std::uniform_real_distribution<Float> dist01(0.0f, 1.0f);

    const Color3 stoneBase{0.46f, 0.44f, 0.40f};
    const Color3 stoneWeathered{0.39f, 0.38f, 0.35f};
    const Color3 mossCap{0.33f, 0.41f, 0.25f};

    // 1. Cracked stone foundation plinth
    addAABB(
        {pos.x() - 0.44f * scale, pos.y(),               pos.z() - 0.44f * scale},
        {pos.x() + 0.44f * scale, pos.y() + 0.14f * scale, pos.z() + 0.44f * scale},
        stoneWeathered, 0.70f, vertices, indices
    );

    // 2. Tall standing pillar (-X, -Z corner)
    Float p1H = (0.85f + dist01(gen) * 0.25f) * scale;
    addAABB(
        {pos.x() - 0.34f * scale, pos.y() + 0.14f * scale, pos.z() - 0.34f * scale},
        {pos.x() - 0.14f * scale, pos.y() + p1H,           pos.z() - 0.14f * scale},
        stoneBase, 0.85f, vertices, indices
    );

    // 3. Second standing pillar (+X, -Z corner)
    Float p2H = (0.75f + dist01(gen) * 0.25f) * scale;
    addAABB(
        {pos.x() + 0.14f * scale, pos.y() + 0.14f * scale, pos.z() - 0.34f * scale},
        {pos.x() + 0.34f * scale, pos.y() + p2H,           pos.z() - 0.14f * scale},
        stoneBase, 0.85f, vertices, indices
    );

    // 4. Moss-covered stone architrave / lintel spanning the two pillars if both are tall
    Float lintelY = std::min(p1H, p2H);
    addAABB(
        {pos.x() - 0.38f * scale, pos.y() + lintelY,               pos.z() - 0.36f * scale},
        {pos.x() + 0.38f * scale, pos.y() + lintelY + 0.14f * scale, pos.z() - 0.12f * scale},
        stoneWeathered, 0.92f, vertices, indices
    );
    addAABB(
        {pos.x() - 0.36f * scale, pos.y() + lintelY + 0.14f * scale, pos.z() - 0.34f * scale},
        {pos.x() + 0.36f * scale, pos.y() + lintelY + 0.18f * scale, pos.z() - 0.14f * scale},
        mossCap, 0.96f, vertices, indices
    );

    // 5. Broken pillar stump (+Z side)
    addAABB(
        {pos.x() - 0.30f * scale, pos.y() + 0.14f * scale, pos.z() + 0.12f * scale},
        {pos.x() - 0.10f * scale, pos.y() + 0.42f * scale, pos.z() + 0.32f * scale},
        stoneWeathered, 0.78f, vertices, indices
    );
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
    const Color3 sootMortarColor{0.18f, 0.17f, 0.16f}; // Victorian coal-ash & dark mortar bed
    const Color3 stoneLight{0.38f, 0.37f, 0.36f};      // Weathered granite sett
    const Color3 stoneDark{0.30f, 0.29f, 0.29f};       // Dark basalt sett
    const Color3 stoneWarm{0.41f, 0.36f, 0.31f};       // Ironstone / porphyry sett
    const Color3 curbColor{0.44f, 0.43f, 0.41f};       // Dressed granite curbstone
    const Float cobbleHeight = 0.024f;

    // Helper to build a miniature Victorian Cast-Iron Gas Streetlamp at (lx, baseY, lz)
    auto addVictorianGasLamp = [&](Float lx, Float baseY, Float lz) {
        const Color3 castIron{0.12f, 0.13f, 0.14f};
        const Color3 brassTrim{0.62f, 0.48f, 0.22f};
        const Color3 gasFlame{1.00f, 0.78f, 0.36f}; // Emissive with AO = 2.0f

        // 1. Octagonal-style stepped cast-iron pedestal base
        addAABB({lx - 0.042f, baseY,          lz - 0.042f},
                {lx + 0.042f, baseY + 0.10f,  lz + 0.042f},
                castIron, 0.65f, vertices, indices);
        addAABB({lx - 0.030f, baseY + 0.10f,  lz - 0.030f},
                {lx + 0.030f, baseY + 0.18f,  lz + 0.030f},
                castIron, 0.75f, vertices, indices);

        // 2. Slender fluted iron post
        addAABB({lx - 0.016f, baseY + 0.18f,  lz - 0.016f},
                {lx + 0.016f, baseY + 0.56f,  lz + 0.016f},
                castIron, 0.85f, vertices, indices);

        // 3. Ladder rest crossbar & brass collar
        addAABB({lx - 0.055f, baseY + 0.48f,  lz - 0.012f},
                {lx + 0.055f, baseY + 0.50f,  lz + 0.012f},
                brassTrim, 0.90f, vertices, indices);

        // 4. Glowing Amber Gas Lantern Cage (AO = 2.0 triggers emissive shader glow!)
        addAABB({lx - 0.038f, baseY + 0.56f,  lz - 0.038f},
                {lx + 0.038f, baseY + 0.67f,  lz + 0.038f},
                gasFlame, 2.0f, vertices, indices);

        // 5. Pitched copper/iron lantern roof & finial
        addAABB({lx - 0.048f, baseY + 0.67f,  lz - 0.048f},
                {lx + 0.048f, baseY + 0.70f,  lz + 0.048f},
                castIron, 0.92f, vertices, indices);
        addAABB({lx - 0.020f, baseY + 0.70f,  lz - 0.020f},
                {lx + 0.020f, baseY + 0.74f,  lz + 0.020f},
                brassTrim, 0.96f, vertices, indices);
    };

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

        // 5. Place a Victorian Cast-Iron Gas Streetlamp at intersections or every 3rd tile along avenues
        Int tileHash = std::abs(Int(std::round(pos.x())) * 3 + Int(std::round(pos.z())) * 7);
        if(branchCount >= 3 || (branchCount >= 1 && (tileHash % 3 == 0))) {
            Float lampX = pos.x() + (hasE ? -0.40f : 0.40f);
            Float lampZ = pos.z() + (hasN ? -0.40f : 0.40f);
            addVictorianGasLamp(lampX, curbTopY, lampZ);
        }
        return;
    }

    // SLOPED RAMP ROAD:
    // Computes elevation per vertex smoothly interpolating from lower edge (0) to upper edge (deltaY)
    auto evalHeight = [&](Float u) -> Float {
        // u is in range [0.0, 1.0] along the ramp direction
        return pos.y() + 0.002f + u * deltaY;
    };

    // Calculate the 4 corners of the sloped roadbed
    // c00 (-X, -Z), c10 (+X, -Z), c11 (+X, +Z), c01 (-X, +Z)
    Float h00, h10, h11, h01;
    if(rampDirX != 0) {
        // Ramp along X axis
        Float uLeft  = (rampDirX > 0) ? 0.0f : 1.0f;
        Float uRight = (rampDirX > 0) ? 1.0f : 0.0f;
        h00 = evalHeight(uLeft);
        h01 = evalHeight(uLeft);
        h10 = evalHeight(uRight);
        h11 = evalHeight(uRight);
    } else {
        // Ramp along Z axis
        Float uBack  = (rampDirZ > 0) ? 0.0f : 1.0f;
        Float uFront = (rampDirZ > 0) ? 1.0f : 0.0f;
        h00 = evalHeight(uBack);
        h10 = evalHeight(uBack);
        h01 = evalHeight(uFront);
        h11 = evalHeight(uFront);
    }

    // Normal for sloped top surface
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
        // Back (-Z) masonry skirt
        base = vertices.size();
        Vector3 bNorm{0.0f, 0.0f, -1.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, bNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() - halfTile}, bNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, bNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, bNorm, curbColor, 0.88f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        // Front (+Z) masonry skirt
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
        // Left (-X) masonry skirt
        base = vertices.size();
        Vector3 lNorm{-1.0f, 0.0f, 0.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, lNorm, curbColor, 0.70f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, lNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, lNorm, curbColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() + halfTile}, lNorm, curbColor, 0.70f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        // Right (+X) masonry skirt
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
