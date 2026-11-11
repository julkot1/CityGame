#include "world/RandomWorldGenerator.h"
#include "world/Chunk.h"
#include <Magnum/Math/Functions.h>
#include <cmath>
#include <random>

namespace Magnum {

RandomWorldGenerator::RandomWorldGenerator(UnsignedInt seed) {
    setSeed(seed);
    initBiomeRules();
}

void RandomWorldGenerator::initBiomeRules() {
    _biomeRules.clear();

    // Adjust forest moisture threshold based on _forestDensity (higher density = lower moisture needed for forest)
    Float forestThreshold = Math::clamp(0.76f - _forestDensity * 0.16f, 0.30f, 0.92f);

    // High altitude snow peaks (warm parchment frost)
    _biomeRules.push_back({TileType::Snow,   0xe6ded0_rgbf, _seaLevel + 9.0f, 100.0f, 0.0f, 1.0f});

    // Mountain rock tops (Victorian dark slate & basalt)
    _biomeRules.push_back({TileType::Rock,   0x52565c_rgbf, _seaLevel + 4.4f, _seaLevel + 9.0f, 0.0f, 1.0f});

    // Highland Heather Moorland (warm russet-olive upland meadows between plains and rock peaks)
    _biomeRules.push_back({TileType::Heath,  0x6b5d44_rgbf, _seaLevel + 1.8f, _seaLevel + 4.4f, 0.0f, forestThreshold});

    // Lush forest (clustered woodland groves in higher-moisture zones)
    _biomeRules.push_back({TileType::Forest, 0x384d36_rgbf, _seaLevel + 0.4f, _seaLevel + 4.4f, forestThreshold, 1.0f});

    // Grass plains (expansive flat building land)
    _biomeRules.push_back({TileType::Grass,  0x566944_rgbf, _seaLevel + 0.4f, _seaLevel + 1.8f, 0.0f, forestThreshold});

    // Shoreline & coastal silt/pebbles
    _biomeRules.push_back({TileType::Sand,   0x8f7d63_rgbf, _seaLevel, _seaLevel + 0.4f, 0.0f, 1.0f});
}

void RandomWorldGenerator::setSeed(UnsignedInt seed) {
    _seed = seed;
    _continentNoise = SimplexNoise{0.0035f, 1.0f, 2.0f, 0.5f};
    _mountainNoise  = SimplexNoise{0.009f, 1.0f, 2.0f, 0.5f};
    _hillNoise      = SimplexNoise{0.02f, 1.0f, 2.0f, 0.5f};
    _riverNoise     = SimplexNoise{0.0055f, 1.0f, 2.0f, 0.5f};
    _tributaryNoise = SimplexNoise{0.011f, 1.0f, 2.0f, 0.5f};
    _moistureNoise  = SimplexNoise{0.015f, 1.0f, 2.0f, 0.5f};
    initBiomeRules();
}

Tile RandomWorldGenerator::classifyBiome(Float elevation, Float moisture, bool isRiver, bool isRiverBank) const {
    Tile t;
    t.active = true;

    if(elevation <= _seaLevel || (isRiver && elevation < _seaLevel + 0.35f)) {
        t.height = _seaLevel;
        t.type = TileType::Water;
        t.color = 0x284b57_rgbf; // Deep turbid canal / Victorian slate-teal water
        return t;
    }

    t.height = elevation;

    // Alluvial Clay Banks along moist riverbanks & estuaries (ideal for 19th-century brickworks)
    if(isRiverBank && elevation <= _seaLevel + 0.65f && moisture > 0.48f) {
        t.type = TileType::Clay;
        t.color = 0x6e4f3b_rgbf; // Rich terracotta-umber alluvial clay
        return t;
    }

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
    t.color = 0x566944_rgbf;
    return t;
}

Tile RandomWorldGenerator::generateTile(Int worldX, Int worldZ) {
    // Seed coordinate offsets
    float sx = static_cast<float>(worldX) + static_cast<float>(_seed % 7919u);
    float sz = static_cast<float>(worldZ) + static_cast<float>((_seed * 31u) % 7919u);

    // 1. Broad continental landmass [0.0 .. 1.0]
    float continent = (_continentNoise.fractal(3, sx, sz) + 1.0f) * 0.5f;

    // 2. Gentle rolling hill noise [0.0 .. 1.0]
    float hills = (_hillNoise.fractal(2, sx + 1200.0f, sz + 1200.0f) + 1.0f) * 0.5f;
    float moisture = (_moistureNoise.fractal(2, sx + 2000.0f, sz + 2000.0f) + 1.0f) * 0.5f;

    // Archetype adjustments to continental distribution
    float coastThreshold = 0.30f;
    float highlandStart  = 0.72f;
    if(_archetype == WorldArchetype::Archipelago) {
        // More islands, channels, harbors, and peninsulas
        coastThreshold = 0.44f;
        highlandStart  = 0.78f;
        continent = continent * 0.75f + hills * 0.25f;
    } else if(_archetype == WorldArchetype::HighlandPlateaus) {
        // Less ocean, more stepped upland moorland mesas & mountain passes
        coastThreshold = 0.20f;
        highlandStart  = 0.56f;
    } else if(_archetype == WorldArchetype::GreatPlains) {
        // Expansive industrial flatlands with minimal ocean and distant mountains
        coastThreshold = 0.18f;
        highlandStart  = 0.84f;
    }

    // 3. City-Builder Basin & Terrace Shaping
    float rawElevation = _seaLevel;
    if(continent < coastThreshold) {
        // Underwater shelf
        float depthT = (coastThreshold - continent) / Math::max(0.05f, coastThreshold);
        rawElevation = _seaLevel - depthT * 2.5f;
    } else if(continent < highlandStart) {
        // Main Buildable Valley & Terraced Plains
        float plainT = (continent - coastThreshold) / Math::max(0.10f, highlandStart - coastThreshold);

        // Flatness blends between continuous rolling slope (flatness = 0) and wide flat terraces (flatness = 1)
        float terraceHeight = 0.6f;
        if(plainT < 0.45f) {
            float ramp = Math::clamp((plainT - 0.38f) / 0.07f, 0.0f, 1.0f);
            terraceHeight = Math::lerp(0.6f, 1.0f, ramp);
        } else if(plainT < 0.82f) {
            terraceHeight = 1.0f;
        } else {
            float ramp = Math::clamp((plainT - 0.82f) / 0.10f, 0.0f, 1.0f);
            terraceHeight = Math::lerp(1.0f, 1.6f, ramp);
        }

        float rollingHeight = 0.4f + plainT * 1.8f + (hills - 0.5f) * 1.2f;
        float basePlainHeight = Math::lerp(rollingHeight, terraceHeight, Math::clamp(_flatness, 0.0f, 1.0f));
        float subtleRelief = (hills - 0.5f) * Math::lerp(0.65f, 0.16f, Math::clamp(_flatness, 0.0f, 1.0f));
        rawElevation = _seaLevel + basePlainHeight + subtleRelief;
    } else {
        // Stepped Highland Moorlands & Dramatic Mountain Ranges
        float highlandT = (continent - highlandStart) / Math::max(0.10f, 1.0f - highlandStart);
        float mRaw = _mountainNoise.fractal(3, sx + 500.0f, sz + 500.0f);
        float ridge = 1.0f - std::abs(mRaw);
        ridge = ridge * ridge;

        // Stepped upland plateau shelf before steep mountain peaks
        float uplandShelf = (highlandT < 0.35f)
            ? Math::lerp(1.6f, 2.4f, Math::clamp(highlandT / 0.15f, 0.0f, 1.0f))
            : (2.4f + (highlandT - 0.35f) * 4.5f * _mountainScale);

        float mountainMask = Math::max(0.0f, (highlandT - 0.25f) / 0.75f);
        mountainMask = mountainMask * mountainMask;
        float mountains = ridge * mountainMask * 9.5f * _mountainScale;
        rawElevation = _seaLevel + uplandShelf + (hills - 0.5f) * 0.9f * _mountainScale + mountains;
    }

    // 4. Primary Rivers + Branching Tributaries & Alluvial Banks
    bool isRiver = false;
    bool isRiverBank = false;
    if(_riverIntensity > 0.05f && rawElevation > _seaLevel) {
        float primaryRiver = std::abs(_riverNoise.fractal(3, sx + 3000.0f, sz + 3000.0f));
        float tributary    = std::abs(_tributaryNoise.fractal(2, sx + 4500.0f, sz + 4500.0f));

        float riverWidth  = 0.048f * _riverIntensity;
        float valleyWidth = 0.17f  * Math::sqrt(_riverIntensity);

        // Tributaries branch into the main river valley
        float combinedRiver = primaryRiver;
        if(primaryRiver < valleyWidth * 1.6f) {
            combinedRiver = Math::min(primaryRiver, tributary * 1.35f);
        }

        if(combinedRiver < valleyWidth) {
            float bankBlend = Math::clamp((combinedRiver - riverWidth) / Math::max(0.01f, valleyWidth - riverWidth), 0.0f, 1.0f);
            bankBlend = bankBlend * bankBlend * (3.0f - 2.0f * bankBlend);
            float embankmentHeight = _seaLevel + 0.6f;
            rawElevation = Math::lerp(embankmentHeight, rawElevation, bankBlend);

            if(combinedRiver < riverWidth) {
                rawElevation = _seaLevel - 0.5f;
                isRiver = true;
            } else if(combinedRiver < riverWidth * 2.1f) {
                isRiverBank = true;
            }
        }
    }

    constexpr float StepSize = 0.2f;
    float elevation = std::round(rawElevation / StepSize) * StepSize;

    return classifyBiome(elevation, moisture, isRiver, isRiverBank);
}

void RandomWorldGenerator::generateRocks(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId) {
    if(t.type == TileType::Water || t.type == TileType::Snow) return;
    if(chunk.entityAt(localX, localZ) != nullptr) return;

    std::seed_seq seedSeq{
        static_cast<uint32_t>(worldX),
        static_cast<uint32_t>(worldZ),
        static_cast<uint32_t>(_seed * 31u)
    };
    std::mt19937 rng(seedSeq);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    float randVal = dist01(rng);
    float rockChance = 0.0f;
    if(t.type == TileType::Rock) {
        rockChance = 0.040f;
    } else if(t.type == TileType::Heath) {
        rockChance = 0.012f; // Scattered moorland standing stones/boulders
    } else if(t.type == TileType::Forest || t.type == TileType::Sand || t.type == TileType::Clay) {
        rockChance = 0.003f;
    } else if(t.type == TileType::Grass) {
        rockChance = 0.0004f; // Keep city-building plains clear
    }

    if(randVal < rockChance) {
        Entity rock;
        rock.id = nextEntityId++;
        rock.type = EntityType::Rock;
        rock.position = Vector3{Float(localX), t.height, Float(localZ)};
        rock.scale = 0.85f + dist01(rng) * 0.35f; 
        chunk.addEntity(rock);
    } 
}

void RandomWorldGenerator::generateTrees(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId) {
    if(_forestDensity <= 0.01f || chunk.entityAt(localX, localZ) != nullptr) return;

    std::seed_seq seedSeq{
        static_cast<uint32_t>(worldX),
        static_cast<uint32_t>(worldZ),
        static_cast<uint32_t>(_seed)
    };
    std::mt19937 rng(seedSeq);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    float randVal = dist01(rng);

    if(t.type == TileType::Forest && randVal < 0.20f * _forestDensity) {
        Entity tree;
        tree.id = nextEntityId++;
        tree.type = (dist01(rng) < 0.65f) ? EntityType::TreeOak : EntityType::TreePine;
        tree.position = Vector3{Float(localX), t.height, Float(localZ)};
        tree.scale = 0.85f + dist01(rng) * 0.35f;
        chunk.addEntity(tree);
    } else if(t.type == TileType::Heath && randVal < 0.022f * _forestDensity) {
        Entity tree;
        tree.id = nextEntityId++;
        tree.type = (dist01(rng) < 0.45f) ? EntityType::TreeOak : EntityType::TreePine;
        tree.position = Vector3{Float(localX), t.height, Float(localZ)};
        tree.scale = 0.78f + dist01(rng) * 0.30f;
        chunk.addEntity(tree);
    } else if(t.type == TileType::Grass && randVal < 0.005f * _forestDensity) {
        Entity tree;
        tree.id = nextEntityId++;
        tree.type = EntityType::TreeOak;
        tree.position = Vector3{Float(localX), t.height, Float(localZ)};
        tree.scale = 0.80f + dist01(rng) * 0.30f;
        chunk.addEntity(tree);
    } else if(t.type == TileType::Rock && t.height < _seaLevel + 8.5f && randVal < 0.02f * _forestDensity) {
        Entity tree;
        tree.id = nextEntityId++;
        tree.type = EntityType::TreePine;
        tree.position = Vector3{Float(localX), t.height, Float(localZ)};
        tree.scale = 0.70f + dist01(rng) * 0.30f;
        chunk.addEntity(tree);
    }
}

void RandomWorldGenerator::generateResources(Chunk& chunk, Tile& t, Int localX, Int localZ, Int worldX, Int worldZ, UnsignedInt& nextEntityId) {
    if(_resourceDensity <= 0.01f || t.type == TileType::Water || t.type == TileType::Snow) return;
    if(chunk.entityAt(localX, localZ) != nullptr) return;

    std::seed_seq seedSeq{
        static_cast<uint32_t>(worldX),
        static_cast<uint32_t>(worldZ),
        static_cast<uint32_t>(_seed * 97u + 13u)
    };
    std::mt19937 rng(seedSeq);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    // Cluster mineral veins using low-frequency mountain noise so ores appear in rich geological seams
    float veinField = _mountainNoise.fractal(2, float(worldX) * 2.2f + 900.0f, float(worldZ) * 2.2f + 900.0f);
    float roll = dist01(rng);

    // 1. Anthracite Coal Seams: appear in upland Heath, Forest foothills, and lower Rock ledges
    if((t.type == TileType::Heath || t.type == TileType::Rock || t.type == TileType::Forest) &&
       veinField > 0.25f && roll < 0.014f * _resourceDensity)
    {
        Entity ore;
        ore.id = nextEntityId++;
        ore.type = EntityType::CoalDeposit;
        ore.position = Vector3{Float(localX), t.height, Float(localZ)};
        ore.scale = 0.85f + dist01(rng) * 0.30f;
        chunk.addEntity(ore);
        return;
    }

    // 2. Iron Ore Deposits: appear on Rock slopes and upper Heath crags
    if((t.type == TileType::Rock || t.type == TileType::Heath) &&
       veinField < -0.20f && roll < 0.016f * _resourceDensity)
    {
        Entity ore;
        ore.id = nextEntityId++;
        ore.type = EntityType::IronDeposit;
        ore.position = Vector3{Float(localX), t.height, Float(localZ)};
        ore.scale = 0.85f + dist01(rng) * 0.32f;
        chunk.addEntity(ore);
        return;
    }

    // 3. Verdigris Copper Veins: appear on Heath moorlands and Rock ridges
    if((t.type == TileType::Heath || t.type == TileType::Rock) &&
       std::abs(veinField) < 0.15f && roll < 0.010f * _resourceDensity)
    {
        Entity ore;
        ore.id = nextEntityId++;
        ore.type = EntityType::CopperDeposit;
        ore.position = Vector3{Float(localX), t.height, Float(localZ)};
        ore.scale = 0.82f + dist01(rng) * 0.28f;
        chunk.addEntity(ore);
        return;
    }

    // 4. Rare Old World Stone Ruins on scenic plateaus and moorlands
    if((t.type == TileType::Heath || t.type == TileType::Grass) && roll < 0.00045f * _resourceDensity) {
        Entity ruins;
        ruins.id = nextEntityId++;
        ruins.type = EntityType::AncientRuins;
        ruins.position = Vector3{Float(localX), t.height, Float(localZ)};
        ruins.scale = 0.90f + dist01(rng) * 0.20f;
        chunk.addEntity(ruins);
    }
}

void RandomWorldGenerator::generateChunk(Chunk& chunk) {
    chunk.clearEntities();
    const Vector2i cc = chunk.chunkCoord();
    UnsignedInt nextEntityId = 1;

    for(Int z = 0; z < Chunk::Size; ++z) {
        for(Int x = 0; x < Chunk::Size; ++x) {
            Int worldX = cc.x() * Chunk::Size + x;
            Int worldZ = cc.y() * Chunk::Size + z;
            Tile t = generateTile(worldX, worldZ);
            chunk.tile(x, z) = t;
            generateResources(chunk, t, x, z, worldX, worldZ, nextEntityId);
            generateTrees(chunk, t, x, z, worldX, worldZ, nextEntityId);
            generateRocks(chunk, t, x, z, worldX, worldZ, nextEntityId);
        }
    }
}

}
