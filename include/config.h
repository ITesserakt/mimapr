#pragma once

#include <Eigen/Sparse>
#include <yaml-cpp/node/node.h>

#include "object.h"

namespace config {

enum class HoleType { Square, Circle };

struct HoleOptions {
    Eigen::Vector2d center;
    HoleType type;

    [[nodiscard]] bool isSquare() const { return type == HoleType::Square; }
    [[nodiscard]] bool isCircle() const { return type == HoleType::Circle; }
};

struct BorderConditions {
    ObjectBound Heat;
    ObjectBound ThermalInsulation;
    ObjectBound Convection;
};

struct TaskParameters {
    HoleOptions hole;
    BorderConditions border;

    static TaskParameters GenerateForVariant(size_t variant);
};

struct Constants {
    int TimeLayers = 100;
    double DeltaTime = 0.1;
    double Height = 400;
    double Width = 500;
    double Radius2 = 150;
    double Radius1 = 50;
    double SquareSide = 100;
    int Variant = 1;
    double GridStep = 5;
    bool OnlyLastTimeLayer = true;
};

} // namespace config

namespace YAML {
using namespace config;

template <> struct convert<config::Constants> {
    static Node encode(const Constants &rhs) {
        Node node;
        Constants default_constants;

#define IfNotDefault(field, name)                                              \
    if (rhs.field != default_constants.field)                                  \
    node[name] = rhs.field

        IfNotDefault(Variant, "variant");
        IfNotDefault(GridStep, "grid_step");
        IfNotDefault(TimeLayers, "time_layers");
        IfNotDefault(DeltaTime, "delta_time");
        IfNotDefault(Height, "height");
        IfNotDefault(Width, "width");
        IfNotDefault(Radius2, "big_radius");
        IfNotDefault(Radius1, "hole_radius");
        IfNotDefault(SquareSide, "square_size");
        IfNotDefault(OnlyLastTimeLayer, "only_last_layer");
        return node;

#undef IfNotDefault
    }

    static bool decode(const Node &node, Constants &rhs) {
        rhs.Variant = node["variant"].as<int>(rhs.Variant);
        rhs.GridStep = node["grid_step"].as<double>(rhs.GridStep);
        rhs.TimeLayers = node["time_layers"].as<int>(rhs.TimeLayers);
        rhs.DeltaTime = node["delta_time"].as<double>(rhs.DeltaTime);
        rhs.Height = node["height"].as<double>(rhs.Height);
        rhs.Width = node["width"].as<double>(rhs.Width);
        rhs.Radius2 = node["big_radius"].as<double>(rhs.Radius2);
        rhs.Radius1 = node["radius_hole"].as<double>(rhs.Radius1);
        rhs.SquareSide = node["square_size"].as<double>(rhs.SquareSide);
        rhs.OnlyLastTimeLayer = node["only_last_layer"].as<bool>(rhs.OnlyLastTimeLayer);

        return true;
    }
};
} // namespace YAML