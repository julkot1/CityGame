#include "world/EntityModelRegistry.h"
#include <Magnum/Math/Color.h>

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

}
