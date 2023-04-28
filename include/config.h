#pragma once

#include "EnumBitmask.h"
#include <Eigen/Dense>

namespace config {

enum class HoleType { Square, Circle };

struct HoleOptions {
    Eigen::Vector2d center;
    HoleType type;
};

enum class ObjectParts : int { Empty = 0, L = 1, R = 2, T = 4, B = 8, R2 = 16, S = 32, R1 = 64 };

DEFINE_BITMASK_OPERATORS(ObjectParts)

static ObjectParts ObjectEx = ObjectParts::L | ObjectParts::R | ObjectParts::T | ObjectParts::B | ObjectParts::R2;
static ObjectParts ObjectIn = ObjectParts::S | ObjectParts::R1;

struct BorderConditions {
    ObjectParts Heat;
    ObjectParts ThermalInsulation;
    ObjectParts Convection;
};

struct TaskParameters {
    HoleOptions hole;
    BorderConditions border;

    static TaskParameters GenerateForVariant(size_t variant);
};

} // namespace config