#pragma once

#include <ostream>
#include <iomanip>
#if USE_OPEN_MP
#include <omp.h>
#endif

#include "ProgressBar.h"
#include "mesh.h"

template <typename T> using Tensor3 = Eigen::VectorX<Eigen::MatrixX<T>>;

struct Solution {
    Tensor3<double> timeMesh;
    double step;
};

class Solver {
    using Index = Eigen::Vector2i;

  private:
    Tensor3<Node> T;
    Tensor3<double> SavedTemperatures;
    double step;
    config::TaskParameters params;
    int SizeT;
    double dt;

    double explicitCentralDifference(const Index &index);
    double applyBorderConvection(const Index &index);
    double applyBorderInsulation(const Index &index);
    void implicitCentralDifference();

    template <config::SolvingMethod Type> void solveNextLayer();

  public:
    Solver(Mesh &&mesh, const config::Constants &consts);

    template <config::SolvingMethod Type = config::SolvingMethod::Explicit> Solution solve();

    [[nodiscard]] Eigen::Vector2d getNormalToBorder(const Index &index, const Node &node) const;
};

template <config::SolvingMethod Type> void Solver::solveNextLayer() {
    using namespace EnumBitmask;

    auto rows = T(0).rows();
    auto cols = T(0).cols();

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            auto &node = T(1)(i, j);
            if (EnumBitmask::contains(params.border.Heat, node.part)) {
                node.t = 100;
                if (node.part == ObjectBound::R2)
                    node.t = 200;
            } else if (EnumBitmask::contains(params.border.Convection, node.part))
                node.t = applyBorderConvection({i, j});
            else if (EnumBitmask::contains(params.border.ThermalInsulation, node.part))
                node.t = applyBorderInsulation({i, j});
            else if (!EnumBitmask::contains(ObjectBounds::Outer, node.part))
                if constexpr (Type == config::SolvingMethod::Explicit)
                    node.t = explicitCentralDifference({i, j});
        }

    if constexpr (Type == config::SolvingMethod::Implicit)
        implicitCentralDifference();

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            T(0)(i, j).t = T(1)(i, j).t;
}

template <config::SolvingMethod Type> Solution Solver::solve() {
    ProgressBar bar{static_cast<float>(SizeT - 1)};
    for (int currentTime = 0; currentTime < SizeT - 1; currentTime++, bar++) {
        if (SavedTemperatures.size() != 1)
            SavedTemperatures(currentTime) = T(0).cast<double>();

        std::cout << bar;
        solveNextLayer<Type>();
    }
    std::cout << "\n";

    if (SavedTemperatures.size() == 1)
        SavedTemperatures(0) = T(0).cast<double>();

    return {std::move(SavedTemperatures), step};
}