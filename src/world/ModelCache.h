#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include "world/Chunk.h"

namespace Magnum {

struct PaletteMaterial {
    Color3 color{0.7f, 0.7f, 0.7f};
    Float ao{0.85f};
};

struct LoadedModel {
    struct FaceBatch {
        std::string materialName;
        Float aoOverride{-1.0f}; // < 0 means use material's default AO
        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<UnsignedInt> indices;
    };

    std::vector<FaceBatch> batches;
    // variantMaps[0] is empty (default materials); variantMaps[i] maps baseMaterial -> replacementMaterial
    std::vector<std::unordered_map<std::string, std::string>> variantMaps;
};

/**
 * Loads and caches glTF 2.0 / GLB 3D models via Magnum's GltfImporter
 * and maps materials to the master `victorian_palette.mtl` library.
 */
class ModelCache {
    public:
        static const PaletteMaterial& getMaterial(const std::string& name);
        static const LoadedModel* getModel(const std::string& relativeAssetPath);

        static bool appendModel(const std::string& relativeAssetPath,
                                const Vector3& pos,
                                Float scale,
                                UnsignedInt orientation,
                                UnsignedInt variantSeed,
                                std::vector<ChunkVertex>& outVertices,
                                std::vector<UnsignedInt>& outIndices);

        static void clearCache();

    private:
        static void ensurePaletteLoaded();
        static std::string resolveAssetPath(const std::string& relativeAssetPath);
        static LoadedModel parseGltfFile(const std::string& resolvedPath);
};

}
