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
    const Float trunkRadius = 0.14f * scale;
    const Float trunkHeight = 1.20f * scale;
    const Color3 trunkColor{0.36f, 0.22f, 0.12f}; // Rich warm oak bark
    const Color3 leafColorLower{0.22f, 0.48f, 0.18f}; // Lush green canopy
    const Color3 leafColorUpper{0.28f, 0.58f, 0.22f};

    // 1. Trunk
    addAABB(
        {pos.x() - trunkRadius, pos.y(), pos.z() - trunkRadius},
        {pos.x() + trunkRadius, pos.y() + trunkHeight, pos.z() + trunkRadius},
        trunkColor, 0.70f, vertices, indices
    );

    // 2. Lower Canopy block (Wide)
    const Float leafRadius = 0.65f * scale;
    const Float leafBaseY = pos.y() + trunkHeight * 0.75f;
    const Float leafMidY = leafBaseY + 0.90f * scale;
    addAABB(
        {pos.x() - leafRadius, leafBaseY, pos.z() - leafRadius},
        {pos.x() + leafRadius, leafMidY, pos.z() + leafRadius},
        leafColorLower, 0.85f, vertices, indices
    );

    // 3. Upper Canopy block (Tapered top)
    const Float topRadius = 0.42f * scale;
    const Float leafTopY = leafMidY + 0.65f * scale;
    addAABB(
        {pos.x() - topRadius, leafMidY, pos.z() - topRadius},
        {pos.x() + topRadius, leafTopY, pos.z() + topRadius},
        leafColorUpper, 0.95f, vertices, indices
    );
}

void EntityModelRegistry::buildPineTree(const Vector3& pos, Float scale,
                                       std::vector<ChunkVertex>& vertices,
                                       std::vector<UnsignedInt>& indices) {
    const Float trunkRadius = 0.12f * scale;
    const Float trunkHeight = 1.60f * scale;
    const Color3 trunkColor{0.30f, 0.18f, 0.10f};      // Dark pine wood
    const Color3 pineGreenTier1{0.12f, 0.32f, 0.18f};  // Deep dark conifer green
    const Color3 pineGreenTier2{0.15f, 0.38f, 0.22f};
    const Color3 pineGreenTier3{0.20f, 0.45f, 0.26f};

    // 1. Slender Trunk
    addAABB(
        {pos.x() - trunkRadius, pos.y(), pos.z() - trunkRadius},
        {pos.x() + trunkRadius, pos.y() + trunkHeight, pos.z() + trunkRadius},
        trunkColor, 0.70f, vertices, indices
    );

    // 2. Conical Foliage Tier 1 (Bottom broad tier)
    Float tier1R = 0.70f * scale;
    Float tier1Y0 = pos.y() + trunkHeight * 0.45f;
    Float tier1Y1 = tier1Y0 + 0.60f * scale;
    addAABB(
        {pos.x() - tier1R, tier1Y0, pos.z() - tier1R},
        {pos.x() + tier1R, tier1Y1, pos.z() + tier1R},
        pineGreenTier1, 0.80f, vertices, indices
    );

    // 3. Conical Foliage Tier 2 (Middle tier)
    Float tier2R = 0.50f * scale;
    Float tier2Y0 = tier1Y1 - 0.10f * scale;
    Float tier2Y1 = tier2Y0 + 0.55f * scale;
    addAABB(
        {pos.x() - tier2R, tier2Y0, pos.z() - tier2R},
        {pos.x() + tier2R, tier2Y1, pos.z() + tier2R},
        pineGreenTier2, 0.88f, vertices, indices
    );

    // 4. Conical Foliage Tier 3 (Pointy top)
    Float tier3R = 0.28f * scale;
    Float tier3Y0 = tier2Y1 - 0.10f * scale;
    Float tier3Y1 = tier3Y0 + 0.50f * scale;
    addAABB(
        {pos.x() - tier3R, tier3Y0, pos.z() - tier3R},
        {pos.x() + tier3R, tier3Y1, pos.z() + tier3R},
        pineGreenTier3, 0.95f, vertices, indices
    );
}


void EntityModelRegistry::buildRock(const Vector3& pos, Float scale, UnsignedInt seed,
                                    std::vector<ChunkVertex>& vertices,
                                    std::vector<UnsignedInt>& indices) {
    // Deterministic PRNG seeded with entity ID and position so rock shape stays constant forever
    uint32_t s = seed ^ static_cast<uint32_t>(pos.x() * 1000.0f) ^ (static_cast<uint32_t>(pos.z() * 1000.0f) << 16);
    std::mt19937 gen(s);

    std::uniform_real_distribution<Float> sizeDist(0.70f, 0.95f);
    std::uniform_real_distribution<Float> colorDist(-0.04f, 0.04f);
    std::uniform_real_distribution<Float> offsetDist(-0.08f, 0.08f);

    const Color3 baseColor{0.44f, 0.44f, 0.46f};

    // Constant rock dimensions for this rock instance
    const Float width = scale * sizeDist(gen);
    const Float height = scale * sizeDist(gen) * 0.65f;
    const Float depth = scale * sizeDist(gen);

    // Main rock body
    const Float ox = offsetDist(gen) * scale;
    const Float oz = offsetDist(gen) * scale;

    auto randomColor = [&]() {
        Float variation = colorDist(gen);

        return Color3{
            std::clamp(baseColor.r() + variation, 0.0f, 1.0f),
            std::clamp(baseColor.g() + variation, 0.0f, 1.0f),
            std::clamp(baseColor.b() + variation, 0.0f, 1.0f)
        };
    };

    // Main body
    addAABB(
        {pos.x() - width * 0.5f + ox, pos.y(),
         pos.z() - depth * 0.5f + oz},
        {pos.x() + width * 0.5f + ox, pos.y() + height,
         pos.z() + depth * 0.5f + oz},
        randomColor(), 0.75f, vertices, indices
    );

    // Upper rock section
    const Float topWidth = width * 0.65f;
    const Float topDepth = depth * 0.70f;

    addAABB(
        {pos.x() - topWidth * 0.5f + ox,
         pos.y() + height * 0.65f,
         pos.z() - topDepth * 0.5f + oz},
        {pos.x() + topWidth * 0.5f + ox,
         pos.y() + height * 1.15f,
         pos.z() + topDepth * 0.5f + oz},
        randomColor(), 0.85f, vertices, indices
    );

    // Small side protrusion
    addAABB(
        {pos.x() - width * 0.55f + ox,
         pos.y() + height * 0.15f,
         pos.z() - depth * 0.25f + oz},
        {pos.x() - width * 0.20f + ox,
         pos.y() + height * 0.65f,
         pos.z() + depth * 0.25f + oz},
        randomColor(), 0.65f, vertices, indices
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
    const Float roadBedThickness = 0.035f;
    const Color3 mudBaseColor{0.30f, 0.22f, 0.15f}; // Dark packed earth/mud
    const Color3 stoneLight{0.46f, 0.44f, 0.42f};
    const Color3 stoneDark{0.38f, 0.36f, 0.34f};
    const Color3 stoneAccent{0.52f, 0.48f, 0.40f}; // Highlight crossing center stone
    const Float cobbleHeight = 0.025f;

    // If there is no elevation difference, emit flat road according to connectivity mask
    if(Math::abs(deltaY) < 0.01f || (rampDirX == 0 && rampDirZ == 0)) {
        // Roadbed foundation
        addAABB(
            {pos.x() - halfTile, pos.y() + 0.002f, pos.z() - halfTile},
            {pos.x() + halfTile, pos.y() + roadBedThickness, pos.z() + halfTile},
            mudBaseColor, 0.85f, vertices, indices
        );

        // Connections: bit 0 = North (+Z), bit 1 = East (+X), bit 2 = South (-Z), bit 3 = West (-X)
        bool hasN = (connectionMask & 1) != 0;
        bool hasE = (connectionMask & 2) != 0;
        bool hasS = (connectionMask & 4) != 0;
        bool hasW = (connectionMask & 8) != 0;
        int branchCount = (hasN ? 1 : 0) + (hasE ? 1 : 0) + (hasS ? 1 : 0) + (hasW ? 1 : 0);

        // If it's a 3-way or 4-way intersection (T-junction or Crossroad), add a distinct central paved square
        if(branchCount >= 3) {
            const Float plazaHalf = 0.16f;
            addAABB(
                {pos.x() - plazaHalf, pos.y() + roadBedThickness, pos.z() - plazaHalf},
                {pos.x() + plazaHalf, pos.y() + roadBedThickness + cobbleHeight + 0.005f, pos.z() + plazaHalf},
                stoneAccent, 1.0f, vertices, indices
            );
        }

        const Float step = 0.25f;
        const Float stoneHalfSize = 0.105f;

        for(int r = 0; r < 4; ++r) {
            for(int c = 0; c < 4; ++c) {
                Float ox = -0.375f + Float(c) * step;
                Float oz = -0.375f + Float(r) * step;
                const Color3& col = ((r + c) % 2 == 0) ? stoneLight : stoneDark;

                addAABB(
                    {pos.x() + ox - stoneHalfSize, pos.y() + roadBedThickness, pos.z() + oz - stoneHalfSize},
                    {pos.x() + ox + stoneHalfSize, pos.y() + roadBedThickness + cobbleHeight, pos.z() + oz + stoneHalfSize},
                    col, 0.95f, vertices, indices
                );
            }
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

    // 1. Emit Sloped Mud Roadbed Top
    UnsignedInt base = vertices.size();
    vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, topNormal, mudBaseColor, 0.88f});
    vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, topNormal, mudBaseColor, 0.88f});
    vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, topNormal, mudBaseColor, 0.88f});
    vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, topNormal, mudBaseColor, 0.88f});

    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 1);
    indices.push_back(base + 0);
    indices.push_back(base + 3);
    indices.push_back(base + 2);

    // Lateral skirting skirt walls for the ramp so the side of the ramp isn't hollow
    // Left/Right or Front/Back sides
    if(rampDirX != 0) {
        // Front (+Z) and Back (-Z) sides of the ramp
        Float yLow = pos.y() + 0.002f;
        // Back (-Z) skirt
        base = vertices.size();
        Vector3 bNorm{0.0f, 0.0f, -1.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, bNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() - halfTile}, bNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, bNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, bNorm, mudBaseColor, 0.88f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        // Front (+Z) skirt
        base = vertices.size();
        Vector3 fNorm{0.0f, 0.0f, 1.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() + halfTile}, fNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, fNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, fNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() + halfTile}, fNorm, mudBaseColor, 0.70f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
    } else {
        Float yLow = pos.y() + 0.002f;
        // Left (-X) skirt
        base = vertices.size();
        Vector3 lNorm{-1.0f, 0.0f, 0.0f};
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() - halfTile}, lNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() - halfTile, h00 + roadBedThickness, pos.z() - halfTile}, lNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, h01 + roadBedThickness, pos.z() + halfTile}, lNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() - halfTile, yLow, pos.z() + halfTile}, lNorm, mudBaseColor, 0.70f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);

        // Right (+X) skirt
        base = vertices.size();
        Vector3 rNorm{1.0f, 0.0f, 0.0f};
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() - halfTile}, rNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, yLow, pos.z() + halfTile}, rNorm, mudBaseColor, 0.70f});
        vertices.push_back({{pos.x() + halfTile, h11 + roadBedThickness, pos.z() + halfTile}, rNorm, mudBaseColor, 0.88f});
        vertices.push_back({{pos.x() + halfTile, h10 + roadBedThickness, pos.z() - halfTile}, rNorm, mudBaseColor, 0.88f});
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
    }

    // 2. Emit 4 Distinct Stepped Paver Tiers (Steps) climbing along the incline
    const Float step = 0.25f;
    const Float stoneHalfSize = 0.105f;
    const Float stepElevationInterval = deltaY / 4.0f; // Each row rises by 1/4th of the height difference

    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c) {
            Float ox = -0.375f + Float(c) * step;
            Float oz = -0.375f + Float(r) * step;

            // Step index along the climbing direction (0, 1, 2, 3)
            Int stepIndex = 0;
            if(rampDirX != 0) {
                stepIndex = (rampDirX > 0) ? c : (3 - c);
            } else {
                stepIndex = (rampDirZ > 0) ? r : (3 - r);
            }

            // Each tier sits flat on its stepped riser level
            Float tierY = pos.y() + 0.002f + Float(stepIndex) * stepElevationInterval + roadBedThickness;
            const Color3& col = ((r + c) % 2 == 0) ? stoneLight : stoneDark;

            addAABB(
                {pos.x() + ox - stoneHalfSize, tierY, pos.z() + oz - stoneHalfSize},
                {pos.x() + ox + stoneHalfSize, tierY + cobbleHeight + stepElevationInterval * 0.4f, pos.z() + oz + stoneHalfSize},
                col, 0.95f, vertices, indices
            );
        }
    }
}


}
