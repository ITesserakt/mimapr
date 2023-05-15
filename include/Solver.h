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
    Eigen::MatrixXd meshCoeffs;
    Eigen::VectorXd meshFreeCoeffs;

    double explicitCentralDifference(const Index &index);
    double applyBorderConvection(const Index &index);
    double applyBorderInsulation(const Index &index);
    void implicitCentralDifference();
    [[nodiscard]] Eigen::MatrixXd buildCoefficientMatrix() const;
    [[nodiscard]] Eigen::VectorXd buildFreeDicksVector() const;

    template <config::SolvingMethod Type> void solveNextLayer();

  public:
    Solver(Mesh &&mesh, const config::Constants &consts);

    Solution solveExplicit();
    Solution solveImplicit();

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