#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>
#include "world/Tile.h"

namespace Magnum {

class Chunk;

/**
 * Interface / Base class for world terrain generators.
 * Allows switching between random generation, Perlin/Simplex noise, biome generators, etc.
 */
class WorldGenerator {
    public:
        virtual ~WorldGenerator() = default;

        /**
         * Generate all tiles for a given chunk based on its world coordinate.
         */
        virtual void generateChunk(Chunk& chunk) = 0;

        /**
         * Query procedural tile parameters at any arbitrary world tile coordinate.
         */
        virtual Tile generateTile(Int worldX, Int worldZ) = 0;
};

}
