#include <array>

#include "config.h"

using namespace config;
using namespace Eigen;

static std::array<HoleType, 2> gamma1 = {HoleType::Square, HoleType::Circle};
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
    auto heat = get_by_id(gamma3, variant);
    auto heatWithR2 = BorderConditions{heat.Heat | ObjectBound::R2,
                                       heat.ThermalInsulation, heat.Convection};
    return TaskParameters{
        HoleOptions{get_by_id(gamma2, variant), get_by_id(gamma1, variant)},
        heatWithR2};
}

ObjectBound BorderConditions::bound() const {
    return Heat | ThermalInsulation | Convection;
}

bool Constants::operator==(const Constants &rhs) const {
    return TimeLayers == rhs.TimeLayers && DeltaTime == rhs.DeltaTime && Height == rhs.Height && Width == rhs.Width &&
           Radius2 == rhs.Radius2 && Radius1 == rhs.Radius1 && SquareSide == rhs.SquareSide && Variant == rhs.Variant &&
           GridStep == rhs.GridStep && Kind == rhs.Kind;
}

bool Constants::operator!=(const Constants &rhs) const { return !(rhs == *this); }

bool Constants::isDefault() const { return *this == Constants{}; }
