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

    [[nodiscard]] ObjectBound bound() const;
};

struct TaskParameters {
    HoleOptions hole;
    BorderConditions border;

    static TaskParameters GenerateForVariant(size_t variant);
};

enum class RenderKind { OutputAll, OutputLast, RenderGif, RenderLast };

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
    RenderKind RenderKind = RenderKind::OutputLast;

    bool isDefault() const;

    bool operator==(const Constants &rhs) const;
    bool operator!=(const Constants &rhs) const;
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
        IfNotDefault(RenderKind, "render_kind");
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
        rhs.RenderKind = node["render_kind"].as<RenderKind>(rhs.RenderKind);

        return true;
    }
};

template <> struct convert<RenderKind> {
    static bool decode(const Node &node, RenderKind &kind) {
        if (!node.IsScalar())
            return false;

        auto value = node.as<std::string>();
        std::transform(value.begin(), value.end(), value.begin(), [](const auto &c) { return std::tolower(c); });
        if (value == "output all")
            kind = config::RenderKind::OutputAll;
        else if (value == "output last")
            kind = config::RenderKind::OutputLast;
        else if (value == "render all")
            kind = config::RenderKind::RenderGif;
        else if (value == "render last")
            kind = config::RenderKind::RenderLast;
        else
            return false;

        return true;
    }

    static Node encode(const RenderKind& kind) {
        Node node;

        if (kind == config::RenderKind::OutputLast)
            node = "output last";
        else if (kind == config::RenderKind::OutputAll)
            node = "output all";
        else if (kind == config::RenderKind::RenderLast)
            node = "render last";
        else if (kind == config::RenderKind::RenderGif)
            node = "render all";

        return node;
    }
};
} // namespace YAML