#include "world/RandomWorldGenerator.h"
#include "world/Chunk.h"
#include <Magnum/Math/Functions.h>
#include <cmath>

namespace Magnum {

RandomWorldGenerator::RandomWorldGenerator(UnsignedInt seed) {
    setSeed(seed);
    initBiomeRules();
}

void RandomWorldGenerator::initBiomeRules() {
    _biomeRules.clear();

    // High altitude snow peaks
    _biomeRules.push_back({TileType::Snow,   0xfbf1c7_rgbf, 14.0f, 100.0f, 0.0f, 1.0f});

    // Mountain rock tops
    _biomeRules.push_back({TileType::Rock,   0x928374_rgbf, 10.5f, 14.0f,  0.0f, 1.0f});

    // Lush forest (higher moisture areas on building plains)
    _biomeRules.push_back({TileType::Forest, 0x427b58_rgbf, _seaLevel + 0.6f, 10.5f, 0.58f, 1.0f});

    // Grass plains (primary building land)
    _biomeRules.push_back({TileType::Grass,  0x689d6a_rgbf, _seaLevel + 0.6f, 10.5f, 0.0f, 0.58f});

    // Shoreline & coastal sand
    _biomeRules.push_back({TileType::Sand,   0xd5c4a1_rgbf, _seaLevel, _seaLevel + 0.6f, 0.0f, 1.0f});
}

void RandomWorldGenerator::setSeed(UnsignedInt seed) {
    _seed = seed;
    _continentNoise = SimplexNoise{0.0035f, 1.0f, 2.0f, 0.5f};
    _mountainNoise  = SimplexNoise{0.009f, 1.0f, 2.0f, 0.5f};
    _hillNoise      = SimplexNoise{0.02f, 1.0f, 2.0f, 0.5f};
    _riverNoise     = SimplexNoise{0.006f, 1.0f, 2.0f, 0.5f};
    _moistureNoise  = SimplexNoise{0.015f, 1.0f, 2.0f, 0.5f};
    initBiomeRules();
}

Tile RandomWorldGenerator::classifyBiome(Float elevation, Float moisture, bool isRiver) const {
    Tile t;
    t.active = true;

    if(elevation <= _seaLevel || (isRiver && elevation < _seaLevel + 0.5f)) {
        t.height = _seaLevel;
        t.type = TileType::Water;
        t.color = isRiver ? 0x458588_rgbf : 0x3d7ea6_rgbf;
        return t;
    }

    t.height = elevation;

    for(const auto& rule : _biomeRules) {
        if(elevation >= rule.minElevation && elevation < rule.maxElevation &&
           moisture  >= rule.minMoisture  && moisture  <= rule.maxMoisture)
        {
            t.type = rule.type;
            t.color = rule.color;
            return t;
        }
    }

    // Default fallback
    t.type = TileType::Grass;
    t.color = 0x689d6a_rgbf;
    return t;
}

Tile RandomWorldGenerator::generateTile(Int worldX, Int worldZ) {
    // Seed coordinate offsets
    float sx = static_cast<float>(worldX) + static_cast<float>(_seed % 7919u);
    float sz = static_cast<float>(worldZ) + static_cast<float>((_seed * 31u) % 7919u);

    float continent = (_continentNoise.fractal(3, sx, sz) + 1.0f) * 0.5f;

    float hills = (_hillNoise.fractal(3, sx + 1200.0f, sz + 1200.0f) + 1.0f) * 0.5f;

    float mRaw = _mountainNoise.fractal(3, sx + 500.0f, sz + 500.0f);
    float ridge = 1.0f - std::abs(mRaw);
    ridge = ridge * ridge;
    float mountainMask = Math::clamp((continent - 0.50f) * 2.5f, 0.0f, 1.0f);
    float mountains = ridge * mountainMask * 8.0f; 

    float rawElevation = (_seaLevel - 2.5f) + continent * 7.5f + (hills - 0.5f) * 3.5f + mountains;

    float riverVal = std::abs(_riverNoise.fractal(3, sx + 3000.0f, sz + 3000.0f));
    constexpr float riverThreshold = 0.065f; 
    bool isRiver = false;
    if(riverVal < riverThreshold && rawElevation > _seaLevel - 1.0f) {
        float riverDepthFactor = 1.0f - (riverVal / riverThreshold);
        rawElevation -= riverDepthFactor * 5.5f;
        isRiver = true;
    }


    constexpr float StepSize = 0.2f;
    float elevation = std::round(rawElevation / StepSize) * StepSize;

    float moisture = (_moistureNoise.fractal(2, sx + 2000.0f, sz + 2000.0f) + 1.0f) * 0.5f;

    return classifyBiome(elevation, moisture, isRiver);
}

void RandomWorldGenerator::generateChunk(Chunk& chunk) {
    const Vector2i cc = chunk.chunkCoord();
    for(Int z = 0; z < Chunk::Size; ++z) {
        for(Int x = 0; x < Chunk::Size; ++x) {
            Int worldX = cc.x() * Chunk::Size + x;
            Int worldZ = cc.y() * Chunk::Size + z;
            chunk.tile(x, z) = generateTile(worldX, worldZ);
        }
    }
}

}
