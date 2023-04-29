#pragma once

#include <Eigen/Dense>

#include "object.h"
#include "EnumBitmask.h"

namespace config {

enum class HoleType { Square, Circle };

struct HoleOptions {
    Eigen::Vector2d center;
    HoleType type;

    [[nodiscard]] bool isSquare() const { return type == HoleType::Square; }
    [[nodiscard]] bool isCircle() const { return type == HoleType::Circle; }
};

struct BorderConditions {
    ObjectBounds Heat;
    ObjectBounds ThermalInsulation;
    ObjectBounds Convection;
};

struct TaskParameters {
    HoleOptions hole;
    BorderConditions border;

    static TaskParameters GenerateForVariant(size_t variant);
};

} // namespace config