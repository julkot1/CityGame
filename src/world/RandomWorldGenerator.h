#pragma once

#include "world/WorldGenerator.h"
#include "world/SimplexNoise.h"
#include <vector>

namespace Magnum {

/**
 * Biome definition rule.
 * Matches elevation and moisture intervals without nested if-else ladders.
 */
struct BiomeRule {
    TileType type{TileType::Grass};
    Color3 color{0x689d6a_rgbf};
    Float minElevation{0.0f};
    Float maxElevation{100.0f};
    Float minMoisture{0.0f};
    Float maxMoisture{1.0f};

    BiomeRule() = default;
    BiomeRule(TileType t, Color3 c, Float minEl, Float maxEl, Float minM = 0.0f, Float maxM = 1.0f):
        type{t}, color{c}, minElevation{minEl}, maxElevation{maxEl}, minMoisture{minM}, maxMoisture{maxM} {}
};

/**
 * Geography archetype preset for world generation.
 */
enum class WorldArchetype: Int {
    RiverBasin = 0,       // Wide alluvial valley with branching rivers & framing mountains
    Archipelago = 1,      // Coastal islands, peninsulas, harbors & sea channels
    HighlandPlateaus = 2, // Stepped mesas, moorlands, and dramatic mountain passes
    GreatPlains = 3       // Vast open industrial plains with gentle rivers
};

/**
 * Procedural multi-noise world generator:
 * - Continental / Base elevation noise (oceans, flatlands, highlands)
 * - Mountain ridge noise (ridged multi-fractal noise)
 * - Hill rolling noise with stepped plateaus
 * - River & tributary channel carving with alluvial clay banks
 * - Data-driven Biome lookup table & 19th-Century mineral resource placement
 */
class RandomWorldGenerator: public WorldGenerator {
    public:
        explicit RandomWorldGenerator(UnsignedInt seed = 1337);

        void setSeed(UnsignedInt seed);
        UnsignedInt seed() const { return _seed; }

        Float seaLevel() const { return _seaLevel; }
        void setSeaLevel(Float level) { _seaLevel = level; initBiomeRules(); }

        WorldArchetype archetype() const { return _archetype; }
        void setArchetype(WorldArchetype arch) { _archetype = arch; }

        Float flatness() const { return _flatness; }
        void setFlatness(Float f) { _flatness = f; }

        Float mountainScale() const { return _mountainScale; }
        void setMountainScale(Float m) { _mountainScale = m; }

        Float riverIntensity() const { return _riverIntensity; }
        void setRiverIntensity(Float r) { _riverIntensity = r; }

        Float forestDensity() const { return _forestDensity; }
        void setForestDensity(Float f) { _forestDensity = f; initBiomeRules(); }

        Float resourceDensity() const { return _resourceDensity; }
        void setResourceDensity(Float r) { _resourceDensity = r; }

        void generateChunk(Chunk& chunk) override;
        Tile generateTile(Int worldX, Int worldZ) override;
        void generateTrees(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId);
        void generateRocks(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId);
        void generateResources(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId);

    private:
        void initBiomeRules();
        Tile classifyBiome(Float elevation, Float moisture, bool isRiver, bool isRiverBank) const;

        UnsignedInt _seed{1337};
        Float _seaLevel{4.5f};
        WorldArchetype _archetype{WorldArchetype::RiverBasin};
        Float _flatness{0.75f};
        Float _mountainScale{1.0f};
        Float _riverIntensity{1.0f};
        Float _forestDensity{1.0f};
        Float _resourceDensity{1.0f};

        // Geological noise layers
        SimplexNoise _continentNoise;
        SimplexNoise _mountainNoise;
        SimplexNoise _hillNoise;
        SimplexNoise _riverNoise;
        SimplexNoise _tributaryNoise;
        SimplexNoise _moistureNoise;

        // Biome lookup table (sorted top-to-bottom or processed sequentially)
        std::vector<BiomeRule> _biomeRules;
};

}
