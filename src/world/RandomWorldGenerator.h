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
 * Procedural multi-noise world generator:
 * - Continental / Base elevation noise (oceans, flatlands, highlands)
 * - Mountain ridge noise (ridged multi-fractal noise)
 * - Hill rolling noise with stepped plateaus
 * - River channel carving
 * - Data-driven Biome lookup table
 */
class RandomWorldGenerator: public WorldGenerator {
    public:
        explicit RandomWorldGenerator(UnsignedInt seed = 1337);

        void setSeed(UnsignedInt seed);
        UnsignedInt seed() const { return _seed; }

        Float seaLevel() const { return _seaLevel; }
        void setSeaLevel(Float level) { _seaLevel = level; }

        void generateChunk(Chunk& chunk) override;
        Tile generateTile(Int worldX, Int worldZ) override;
        void generateTrees(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId);
        void generateRocks(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId);

    private:
        void initBiomeRules();
        Tile classifyBiome(Float elevation, Float moisture, bool isRiver) const;

        UnsignedInt _seed{1337};
        Float _seaLevel{4.5f};

        // Geological noise layers
        SimplexNoise _continentNoise;
        SimplexNoise _mountainNoise;
        SimplexNoise _hillNoise;
        SimplexNoise _riverNoise;
        SimplexNoise _moistureNoise;

        // Biome lookup table (sorted top-to-bottom or processed sequentially)
        std::vector<BiomeRule> _biomeRules;
};

}
