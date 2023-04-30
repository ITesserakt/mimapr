#include <array>

#include "config.h"

using namespace config;
using namespace Eigen;

static std::array<HoleType, 2> gamma1 = {HoleType::Circle, HoleType::Square};
static std::array<Vector2d, 5> gamma2 = {
    Vector2d{155, 155}, Vector2d{155, 255}, Vector2d{355, 255},
    Vector2d{355, 155}, Vector2d{255, 205},
};

using namespace EnumBitmask;
using enum ObjectBound;
using namespace ObjectBounds;

static std::array<BorderConditions, 13> gamma3 = {
    BorderConditions{Ex, Empty, In},
    BorderConditions{In, Empty, Ex},
    BorderConditions{L, In | T | B, R},
    BorderConditions{R, In | T | B, L},
    BorderConditions{T, In | L | R, B},
    BorderConditions{B, In | L | R, T},
    BorderConditions{L | R, In, T | B},
    BorderConditions{L | R | In, Empty, T | B},
    BorderConditions{T | B, In, L | R},
    BorderConditions{T | B | In, Empty, L | R},
    BorderConditions{L | R, Empty, In | T | B},
    BorderConditions{T | B, Empty, In | L | R},
    BorderConditions{L | B, In, T | R},
};

template <typename T, size_t N>
static T get_by_id(std::array<T, N> array, size_t variant) {
    return array[variant % N];
}

TaskParameters TaskParameters::GenerateForVariant(size_t variant) {
    return TaskParameters{
        HoleOptions{get_by_id(gamma2, variant), get_by_id(gamma1, variant)},
        get_by_id(gamma3, variant)};
}
