#include <array>

#include "config.h"

using namespace config;
using namespace Eigen;

static std::array<HoleType, 2> gamma1 = {HoleType::Circle, HoleType::Square};
static std::array<Vector2d, 5> gamma2 = {
    Vector2d{155, 155}, Vector2d{155, 255}, Vector2d{355, 255}, Vector2d{355, 155}, Vector2d{255, 205},
};
static std::array<BorderConditions, 13> gamma3 = {
    BorderConditions{ObjectEx, ObjectParts::Empty, ObjectIn},
    BorderConditions{ObjectIn, ObjectParts::Empty, ObjectEx},
    BorderConditions{ObjectParts::L, ObjectIn | ObjectParts::T | ObjectParts::B, ObjectParts::R},
    BorderConditions{ObjectParts::R, ObjectIn | ObjectParts::T | ObjectParts::B, ObjectParts::L},
    BorderConditions{ObjectParts::T, ObjectIn | ObjectParts::L | ObjectParts::R, ObjectParts::B},
    BorderConditions{ObjectParts::B, ObjectIn | ObjectParts::L | ObjectParts::R, ObjectParts::T},
    BorderConditions{ObjectParts::L | ObjectParts::R, ObjectIn, ObjectParts::T | ObjectParts::B},
    BorderConditions{ObjectParts::L | ObjectParts::R | ObjectIn, ObjectParts::T | ObjectParts::B},
    BorderConditions{ObjectParts::T | ObjectParts::B, ObjectIn, ObjectParts::L | ObjectParts::R},
    BorderConditions{ObjectParts::T | ObjectParts::B | ObjectIn, ObjectParts::L | ObjectParts::R},
    BorderConditions{ObjectParts::L | ObjectParts::R, ObjectIn | ObjectParts::T | ObjectParts::B},
    BorderConditions{ObjectParts::T | ObjectParts::B, ObjectIn | ObjectParts::L | ObjectParts::R},
    BorderConditions{ObjectParts::L | ObjectParts::B, ObjectIn, ObjectParts::T | ObjectParts::R},
};

template <typename T, size_t N> static T get_by_id(std::array<T, N> array, size_t variant) {
    return array[variant % N];
}

TaskParameters TaskParameters::GenerateForVariant(size_t variant) {
    return TaskParameters{HoleOptions{get_by_id(gamma2, variant), get_by_id(gamma1, variant)},
                          get_by_id(gamma3, variant)};
}
