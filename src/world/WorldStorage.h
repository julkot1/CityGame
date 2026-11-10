#pragma once

#include <string>
#include <Magnum/Magnum.h>

namespace Magnum {

class World;

/**
 * Handles serialization, persistence, and loading of the World to/from disk.
 */
class WorldStorage {
    public:
        static bool saveToFile(const World& world, const std::string& filepath = "world.save");
        static bool loadFromFile(World& world, const std::string& filepath = "world.save");
        static void loadOrGenerate(World& world, const std::string& filepath = "world.save");
};

}
