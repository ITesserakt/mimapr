#pragma once

#include "EnumBitmask.h"

enum class ObjectBounds : int {
    Empty = 0,
    L = 1,
    R = 2,
    T = 4,
    B = 8,
    R2 = 16,
    S = 32,
    R1 = 64,
    CircleOuter = 128,
    SquareOuter = 256,
    Inner = 512
};

DEFINE_BITMASK_OPERATORS(ObjectBounds)

static constexpr ObjectBounds ObjectEx =
    ObjectBounds::L | ObjectBounds::R | ObjectBounds::T | ObjectBounds::B | ObjectBounds::R2;
static constexpr ObjectBounds ObjectIn = ObjectBounds::S | ObjectBounds::R1;
static constexpr ObjectBounds Outer = ObjectBounds::CircleOuter | ObjectBounds::SquareOuter;