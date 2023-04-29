#include <array>

#include "config.h"

using namespace config;
using namespace Eigen;

static std::array<HoleType, 2> gamma1 = {HoleType::Circle, HoleType::Square};
static std::array<Vector2d, 5> gamma2 = {
    Vector2d{155, 155}, Vector2d{155, 255}, Vector2d{355, 255}, Vector2d{355, 155}, Vector2d{255, 205},
};
static std::array<BorderConditions, 13> gamma3 = {
    BorderConditions{ObjectEx, ObjectBounds::Empty, ObjectIn},
    BorderConditions{ObjectIn, ObjectBounds::Empty, ObjectEx},
    BorderConditions{ObjectBounds::L, ObjectIn | ObjectBounds::T | ObjectBounds::B, ObjectBounds::R},
    BorderConditions{ObjectBounds::R, ObjectIn | ObjectBounds::T | ObjectBounds::B, ObjectBounds::L},
    BorderConditions{ObjectBounds::T, ObjectIn | ObjectBounds::L | ObjectBounds::R, ObjectBounds::B},
    BorderConditions{ObjectBounds::B, ObjectIn | ObjectBounds::L | ObjectBounds::R, ObjectBounds::T},
    BorderConditions{ObjectBounds::L | ObjectBounds::R, ObjectIn, ObjectBounds::T | ObjectBounds::B},
    BorderConditions{ObjectBounds::L | ObjectBounds::R | ObjectIn, ObjectBounds::Empty, ObjectBounds::T | ObjectBounds::B},
    BorderConditions{ObjectBounds::T | ObjectBounds::B, ObjectIn, ObjectBounds::L | ObjectBounds::R},
    BorderConditions{ObjectBounds::T | ObjectBounds::B | ObjectIn, ObjectBounds::Empty, ObjectBounds::L | ObjectBounds::R},
    BorderConditions{ObjectBounds::L | ObjectBounds::R, ObjectBounds::Empty, ObjectIn | ObjectBounds::T | ObjectBounds::B},
    BorderConditions{ObjectBounds::T | ObjectBounds::B, ObjectBounds::Empty, ObjectIn | ObjectBounds::L | ObjectBounds::R},
    BorderConditions{ObjectBounds::L | ObjectBounds::B, ObjectIn, ObjectBounds::T | ObjectBounds::R},
};

template <typename T, size_t N> static T get_by_id(std::array<T, N> array, size_t variant) {
    return array[variant % N];
}

TaskParameters TaskParameters::GenerateForVariant(size_t variant) {
    return TaskParameters{HoleOptions{get_by_id(gamma2, variant), get_by_id(gamma1, variant)},
                          get_by_id(gamma3, variant)};
}
