#pragma once

#include "EnumBitmask.h"

enum class ObjectBound : int {
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

namespace ObjectBounds {
using namespace EnumBitmask;

static constexpr ObjectBound Ex =
    ObjectBound::L | ObjectBound::R | ObjectBound::T | ObjectBound::B | ObjectBound::R2;
static constexpr ObjectBound In = ObjectBound::S | ObjectBound::R1;
static constexpr ObjectBound Outer = ObjectBound::CircleOuter | ObjectBound::SquareOuter;
static constexpr ObjectBound Max = Ex | In | Outer | ObjectBound::Inner | ObjectBound::S | ObjectBound::R1;
} // namespace ObjectBounds