#pragma once

#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include "world/Entity.h"
#include "world/Chunk.h"

namespace Magnum {

/**
 * Generates low-poly voxel geometry for entity archetypes (Trees, etc.)
 * directly into vertex and index buffers.
 */
class EntityModelRegistry {
    public:
        // Appends entity geometry to the target vertex & index buffers
        static void buildEntityMesh(const Entity& entity,
                                   std::vector<ChunkVertex>& vertices,
                                   std::vector<UnsignedInt>& indices);

        // Individual tree builders
        static void buildOakTree(const Vector3& pos, Float scale,
                                 std::vector<ChunkVertex>& vertices,
                                 std::vector<UnsignedInt>& indices);

        static void buildPineTree(const Vector3& pos, Float scale,
                                  std::vector<ChunkVertex>& vertices,
                                  std::vector<UnsignedInt>& indices);
        static void buildRock(const Vector3& pos, Float scale, UnsignedInt seed,
                              std::vector<ChunkVertex>& vertices,
                              std::vector<UnsignedInt>& indices);
        static void buildOreDeposit(const Vector3& pos, Float scale, UnsignedInt seed,
                                    EntityType oreType,
                                    std::vector<ChunkVertex>& vertices,
                                    std::vector<UnsignedInt>& indices);
        static void buildAncientRuins(const Vector3& pos, Float scale, UnsignedInt seed,
                                      std::vector<ChunkVertex>& vertices,
                                      std::vector<UnsignedInt>& indices);
        static void buildCobblestoneRoad(const Vector3& pos,
                                        std::vector<ChunkVertex>& vertices,
                                        std::vector<UnsignedInt>& indices,
                                        UnsignedByte connectionMask = 0,
                                        Float deltaY = 0.0f,
                                        Int rampDirX = 0,
                                        Int rampDirZ = 0);

    private:
        // Helper to append a colored 3D axis-aligned box
        static void addAABB(const Vector3& minCorner, const Vector3& maxCorner,
                            const Color3& color, Float baseAO,
                            std::vector<ChunkVertex>& vertices,
                            std::vector<UnsignedInt>& indices);
};

}
