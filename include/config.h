#pragma once

#include <Eigen/Sparse>
#include <yaml-cpp/node/node.h>

#include "object.h"

#include <thread>
#include <utility>

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
  private:
    TaskParameters(HoleOptions hole, const BorderConditions &border) : hole(std::move(hole)), border(border) {}

  public:
    HoleOptions hole;
    BorderConditions border;

    static TaskParameters GenerateForVariant(size_t variant);
};

enum class RenderKind { OutputAll, OutputLast, RenderGif, RenderLast, RenderVideo, NoOutput };
enum class SolvingMethod { Explicit, Implicit };

struct Constants {
    int TimeLayers = 100;
    double DeltaTime = 0.1;
    unsigned int Height = 400;
    unsigned int Width = 500;
    double Radius2 = 150;
    double Radius1 = 50;
    double SquareSide = 100;
    int Variant = 1;
    double GridStep = 5;
    bool ExportMeshOnly = false;
    unsigned int Parallelism = std::thread::hardware_concurrency();
    RenderKind Kind = RenderKind::OutputLast;
    SolvingMethod SolveMethod = SolvingMethod::Explicit;

    [[nodiscard]] bool isDefault() const;

    bool operator==(const Constants &rhs) const;
    bool operator!=(const Constants &rhs) const;
};

} // namespace config

namespace YAML {
using namespace config;

template <> struct convert<Constants> {
    static Node encode(const Constants &rhs) {
        Node node;

#define IfNotDefault(field, name) node[name] = rhs.field

        IfNotDefault(Variant, "variant");
        IfNotDefault(GridStep, "grid_step");
        IfNotDefault(TimeLayers, "time_layers");
        IfNotDefault(DeltaTime, "delta_time");
        IfNotDefault(Height, "height");
        IfNotDefault(Width, "width");
        IfNotDefault(Radius2, "big_radius");
        IfNotDefault(Radius1, "hole_radius");
        IfNotDefault(SquareSide, "square_size");
        IfNotDefault(Kind, "render_kind");
        IfNotDefault(SolveMethod, "solving_method");
        IfNotDefault(ExportMeshOnly, "export_mesh_only");
        IfNotDefault(Parallelism, "parallelism");
        return node;

#undef IfNotDefault
    }

    static bool decode(const Node &node, Constants &rhs) {
        rhs.Variant = node["variant"].as<int>(rhs.Variant);
        rhs.GridStep = node["grid_step"].as<double>(rhs.GridStep);
        rhs.TimeLayers = node["time_layers"].as<int>(rhs.TimeLayers);
        rhs.DeltaTime = node["delta_time"].as<double>(rhs.DeltaTime);
        rhs.Height = node["height"].as<decltype(rhs.Height)>(rhs.Height);
        rhs.Width = node["width"].as<decltype(rhs.Width)>(rhs.Width);
        rhs.Radius2 = node["big_radius"].as<double>(rhs.Radius2);
        rhs.Radius1 = node["radius_hole"].as<double>(rhs.Radius1);
        rhs.SquareSide = node["square_size"].as<double>(rhs.SquareSide);
        rhs.Kind = node["render_kind"].as<RenderKind>(rhs.Kind);
        rhs.SolveMethod = node["solving_method"].as<SolvingMethod>(rhs.SolveMethod);
        rhs.ExportMeshOnly = node["export_mesh_only"].as<bool>(rhs.ExportMeshOnly);
        rhs.Parallelism = node["parallelism"].as<unsigned int>(rhs.Parallelism);

        return true;
    }
};

template <> struct convert<RenderKind> {
    static bool decode(const Node &node, RenderKind &kind) {
        if (!node.IsScalar())
            return false;

        const auto value = node.as<std::string>();
        if (value == "output all")
            kind = RenderKind::OutputAll;
        else if (value == "output last")
            kind = RenderKind::OutputLast;
        else if (value == "render all")
            kind = RenderKind::RenderGif;
        else if (value == "render last")
            kind = RenderKind::RenderLast;
        else if (value == "render video")
            kind = RenderKind::RenderVideo;
        else if (value == "no output")
            kind = RenderKind::NoOutput;
        else
            return false;

        return true;
    }

    static Node encode(const RenderKind &kind) {
        switch (kind) {
        case RenderKind::OutputAll:
            return Node{"output all"};
        case RenderKind::OutputLast:
            return Node{"output last"};
        case RenderKind::RenderGif:
            return Node{"render all"};
        case RenderKind::RenderLast:
            return Node{"render last"};
        case RenderKind::RenderVideo:
            return Node{"render video"};
        case RenderKind::NoOutput:
            return Node{"no output"};
        default:
            return Node{"unknown"};
        }
    }
};

template <> struct convert<SolvingMethod> {
    static bool decode(const Node &node, SolvingMethod &method) {
        if (!node.IsScalar())
            return false;

        auto value = node.as<std::string>();
        if (value == "explicit")
            method = SolvingMethod::Explicit;
        else if (value == "implicit")
            method = SolvingMethod::Implicit;
        else
            return false;
        return true;
    }

    static Node encode(const SolvingMethod &method) {
        Node node;
        if (method == SolvingMethod::Explicit)
            node = "explicit";
        else if (method == SolvingMethod::Implicit)
            node = "implicit";

        return node;
    }
};
} // namespace YAML
