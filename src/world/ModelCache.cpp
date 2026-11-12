#include "world/ModelCache.h"
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Utility/Debug.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MaterialData.h>
#include <Magnum/Math/Functions.h>
#include <fstream>
#include <sstream>

namespace Magnum {

namespace {
    bool gPaletteLoaded = false;
    std::unordered_map<std::string, PaletteMaterial> gMaterials;
    std::unordered_map<std::string, LoadedModel> gModels;
    const PaletteMaterial kDefaultMaterial{Color3{0.65f, 0.65f, 0.65f}, 0.85f};

    Vector3 rotateYQuarterTurns(const Vector3& v, UnsignedInt rot) {
        switch(rot & 3u) {
            case 1u: return Vector3{ v.z(), v.y(), -v.x()}; // 90 deg (+Z -> +X)
            case 2u: return Vector3{-v.x(), v.y(), -v.z()}; // 180 deg (+Z -> -Z)
            case 3u: return Vector3{-v.z(), v.y(),  v.x()}; // 270 deg (+Z -> -X)
            default: return v;                              // 0 deg (+Z)
        }
    }
}

std::string ModelCache::resolveAssetPath(const std::string& relativeAssetPath) {
    const std::string prefixes[] = {"", "../", "../../"};
    for(const auto& prefix : prefixes) {
        std::string candidate = prefix + relativeAssetPath;
        std::ifstream f(candidate);
        if(f.good()) {
            return candidate;
        }
    }
    return relativeAssetPath;
}

void ModelCache::ensurePaletteLoaded() {
    if(gPaletteLoaded) return;
    gPaletteLoaded = true;

    std::string path = resolveAssetPath("assets/materials/victorian_palette.mtl");
    std::ifstream in(path);
    if(!in.is_open()) {
        Debug{} << "[ModelCache] Warning: Could not open palette:" << path.c_str();
        return;
    }

    std::string currentMat;
    std::string line;
    while(std::getline(in, line)) {
        if(line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if(cmd == "newmtl") {
            iss >> currentMat;
            gMaterials[currentMat] = kDefaultMaterial;
        } else if(cmd == "Kd" && !currentMat.empty()) {
            Float r = 0.7f, g = 0.7f, b = 0.7f;
            iss >> r >> g >> b;
            gMaterials[currentMat].color = Color3{r, g, b};
        } else if((cmd == "ao" || cmd == "Ns") && !currentMat.empty()) {
            Float ao = 0.85f;
            iss >> ao;
            gMaterials[currentMat].ao = ao;
        }
    }
}

const PaletteMaterial& ModelCache::getMaterial(const std::string& name) {
    ensurePaletteLoaded();
    auto it = gMaterials.find(name);
    if(it != gMaterials.end()) {
        return it->second;
    }
    return kDefaultMaterial;
}

const LoadedModel* ModelCache::getModel(const std::string& relativeAssetPath) {
    ensurePaletteLoaded();
    auto it = gModels.find(relativeAssetPath);
    if(it != gModels.end()) {
        return &it->second;
    }

    std::string resolved = resolveAssetPath(relativeAssetPath);
    LoadedModel loaded = parseGltfFile(resolved);
    auto inserted = gModels.emplace(relativeAssetPath, std::move(loaded));
    return &inserted.first->second;
}

LoadedModel ModelCache::parseGltfFile(const std::string& resolvedPath) {
    LoadedModel model;
    model.variantMaps.emplace_back();

    PluginManager::Manager<Trade::AbstractImporter> manager;
    Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("GltfImporter");
    if(!importer) {
        importer = manager.loadAndInstantiate("AnySceneImporter");
    }
    if(!importer) {
        Debug{} << "[ModelCache] Neither GltfImporter nor AnySceneImporter could be instantiated!";
        return model;
    }

    if(!importer->openFile(resolvedPath)) {
        Debug{} << "[ModelCache] Failed to open glTF/GLB scene file:" << resolvedPath.c_str();
        return model;
    }

    // Load materials from glTF if present
    std::vector<std::string> materialNames;
    for(UnsignedInt mi = 0; mi < importer->materialCount(); ++mi) {
        Containers::String name = importer->materialName(mi);
        std::string matNameStr = !name.isEmpty() ? std::string{name} : ("Material_" + std::to_string(mi));
        materialNames.push_back(matNameStr);

        if(gMaterials.find(matNameStr) == gMaterials.end()) {
            auto matData = importer->material(mi);
            Color3 matColor{0.75f, 0.75f, 0.75f};
            if(matData) {
                if(matData->hasAttribute(Trade::MaterialAttribute::BaseColor)) {
                    matColor = matData->attribute<Color4>(Trade::MaterialAttribute::BaseColor).rgb();
                } else if(matData->hasAttribute(Trade::MaterialAttribute::DiffuseColor)) {
                    matColor = matData->attribute<Color4>(Trade::MaterialAttribute::DiffuseColor).rgb();
                }
            }
            gMaterials[matNameStr] = PaletteMaterial{matColor, 0.85f};
        }
    }

    // Configure style variants for Victorian buildings & flora
    if(resolvedPath.find("house_2x2") != std::string::npos ||
       resolvedPath.find("house_standalone_2x2") != std::string::npos) {
        model.variantMaps.resize(3);
        model.variantMaps[1]["Brick_Terracotta_Lower"] = "Brick_Terracotta_AltLower";
        model.variantMaps[1]["Brick_Terracotta_Upper"] = "Stucco_Cream";
        model.variantMaps[2]["Brick_Terracotta_Lower"] = "Yorkstone_Lower";
        model.variantMaps[2]["Brick_Terracotta_Upper"] = "Yorkstone_Upper";
    } else if(resolvedPath.find("city_center_2x3") != std::string::npos) {
        model.variantMaps.resize(3);
        model.variantMaps[1]["Stucco_Cream"] = "Yorkstone_Upper";
        model.variantMaps[1]["Yorkstone_Lower"] = "Brick_Terracotta_Lower";
        model.variantMaps[2]["Stucco_Cream"] = "Stone_Plinth";
        model.variantMaps[2]["Yorkstone_Lower"] = "Stone_Plinth";
    } else if(resolvedPath.find("warehouse_4x4") != std::string::npos) {
        model.variantMaps.resize(3);
        model.variantMaps[1]["Brick_Terracotta_Lower"] = "Brick_Terracotta_AltLower";
        model.variantMaps[1]["Brick_Terracotta_Upper"] = "Brick_Terracotta_Lower";
        model.variantMaps[2]["Brick_Terracotta_Lower"] = "Yorkstone_Lower";
        model.variantMaps[2]["Brick_Terracotta_Upper"] = "Yorkstone_Upper";
    } else if(resolvedPath.find("tree_oak") != std::string::npos) {
        model.variantMaps.resize(4);
        model.variantMaps[1]["Foliage_Olive_Base"] = "Foliage_Russet_Base";
        model.variantMaps[1]["Foliage_Olive_Mid"] = "Foliage_Russet_Mid";
        model.variantMaps[1]["Foliage_Olive_Crown"] = "Foliage_Russet_Crown";
        model.variantMaps[2]["Foliage_Olive_Base"] = "Foliage_Ochre_Base";
        model.variantMaps[2]["Foliage_Olive_Mid"] = "Foliage_Ochre_Mid";
        model.variantMaps[2]["Foliage_Olive_Crown"] = "Foliage_Ochre_Crown";
        model.variantMaps[3]["Foliage_Olive_Base"] = "Foliage_Sage_Base";
        model.variantMaps[3]["Foliage_Olive_Mid"] = "Foliage_Sage_Mid";
        model.variantMaps[3]["Foliage_Olive_Crown"] = "Foliage_Sage_Crown";
    }

    // Iterate through all meshes in the glTF file
    for(UnsignedInt m = 0; m < importer->meshCount(); ++m) {
        auto meshDataOpt = importer->mesh(m);
        if(!meshDataOpt) continue;
        const Trade::MeshData& meshData = *meshDataOpt;

        if(!meshData.hasAttribute(Trade::MeshAttribute::Position)) continue;

        std::string batchMat = "Stone_Plinth";
        Containers::String meshNameStr = importer->meshName(m);
        if(!meshNameStr.isEmpty()) {
            batchMat = std::string{meshNameStr};
        } else if(m < materialNames.size()) {
            batchMat = materialNames[m];
        }

        LoadedModel::FaceBatch batch;
        batch.materialName = batchMat;
        batch.aoOverride = -1.0f;

        Containers::Array<Vector3> positions = meshData.positions3DAsArray();
        batch.positions.assign(positions.begin(), positions.end());

        if(meshData.hasAttribute(Trade::MeshAttribute::Normal)) {
            Containers::Array<Vector3> normals = meshData.normalsAsArray();
            batch.normals.assign(normals.begin(), normals.end());
        } else {
            batch.normals.resize(batch.positions.size(), Vector3{0.0f, 1.0f, 0.0f});
        }

        if(meshData.isIndexed()) {
            Containers::Array<UnsignedInt> indices = meshData.indicesAsArray();
            batch.indices.assign(indices.begin(), indices.end());
        } else {
            batch.indices.resize(batch.positions.size());
            for(std::size_t i = 0; i < batch.positions.size(); ++i) {
                batch.indices[i] = static_cast<UnsignedInt>(i);
            }
        }

        // If normals were missing, compute flat face normals
        if(!meshData.hasAttribute(Trade::MeshAttribute::Normal)) {
            for(std::size_t i = 0; i + 2 < batch.indices.size(); i += 3) {
                UnsignedInt i0 = batch.indices[i + 0];
                UnsignedInt i1 = batch.indices[i + 1];
                UnsignedInt i2 = batch.indices[i + 2];
                if(i0 < batch.positions.size() && i1 < batch.positions.size() && i2 < batch.positions.size()) {
                    Vector3 fn = Math::cross(batch.positions[i1] - batch.positions[i0],
                                             batch.positions[i2] - batch.positions[i0]).normalized();
                    batch.normals[i0] = fn;
                    batch.normals[i1] = fn;
                    batch.normals[i2] = fn;
                }
            }
        }

        model.batches.push_back(std::move(batch));
    }

    return model;
}

bool ModelCache::appendModel(const std::string& relativeAssetPath,
                             const Vector3& pos,
                             Float scale,
                             UnsignedInt orientation,
                             UnsignedInt variantSeed,
                             std::vector<ChunkVertex>& outVertices,
                             std::vector<UnsignedInt>& outIndices) {
    const LoadedModel* model = getModel(relativeAssetPath);
    if(!model || model->batches.empty()) {
        return false;
    }

    const UnsignedInt variantIdx = model->variantMaps.empty()
        ? 0u
        : (variantSeed % static_cast<UnsignedInt>(model->variantMaps.size()));
    const auto& variantMap = model->variantMaps[variantIdx];

    for(const auto& batch : model->batches) {
        std::string matName = batch.materialName;
        auto varIt = variantMap.find(matName);
        if(varIt != variantMap.end()) {
            matName = varIt->second;
        }

        const PaletteMaterial& mat = getMaterial(matName);
        const Float ao = (batch.aoOverride >= 0.0f) ? batch.aoOverride : mat.ao;
        const Color3 color = mat.color;

        UnsignedInt baseVertex = static_cast<UnsignedInt>(outVertices.size());
        for(std::size_t i = 0; i < batch.positions.size(); ++i) {
            Vector3 rotatedPos = rotateYQuarterTurns(batch.positions[i], orientation) * scale + pos;
            Vector3 rotatedNorm = rotateYQuarterTurns(batch.normals[i], orientation);
            outVertices.push_back(ChunkVertex{rotatedPos, rotatedNorm, color, ao});
        }

        for(UnsignedInt idx : batch.indices) {
            outIndices.push_back(baseVertex + idx);
        }
    }

    return true;
}

void ModelCache::clearCache() {
    gPaletteLoaded = false;
    gMaterials.clear();
    gModels.clear();
}

}
