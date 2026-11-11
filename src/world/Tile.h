#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

using namespace Math::Literals;

enum class TileType: UnsignedByte {
    Empty = 0,
    Water,
    Sand,
    Clay,
    Grass,
    Heath,
    Forest,
    Rock,
    Snow,
    Road
};

struct Tile {
    TileType type{TileType::Grass};
    Color3 color{0x689d6a_rgbf};
    Float height{1.0f};
    bool active{true};
};

}
