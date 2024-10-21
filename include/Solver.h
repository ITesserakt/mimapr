#pragma once

#include "mesh.h"

template <typename T> using Tensor3 = Eigen::VectorX<Eigen::MatrixX<T>>;

struct Solution {
    Tensor3<float> timeMesh;
    double step;
};

class Solver {
    using Index = Eigen::Vector2i;

    Tensor3<Node> T;
    Tensor3<float> SavedTemperatures;
    double step;
    config::TaskParameters params;
    int SizeT;
    double dt;
    Eigen::MatrixXd meshCoeffs;
    Eigen::VectorXd meshFreeCoeffs;

    [[nodiscard]] double explicitCentralDifference(const Index &index) const;
    [[nodiscard]] double applyBorderConvection(const Index &index) const;
    [[nodiscard]] double applyBorderInsulation(const Index &index) const ;
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

    const auto rows = T(0).rows();
    const auto cols = T(0).cols();

#pragma omp parallel for default(none) shared(rows, cols)
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            auto &[t, part, _] = T(1)(i, j);
            if (contains(params.border.Heat, part)) {
                t = 100;
                if (part == ObjectBound::R2)
                    t = 200;
            } else if (contains(params.border.Convection, part))
                t = applyBorderConvection({i, j});
            else if (contains(params.border.ThermalInsulation, part))
                t = applyBorderInsulation({i, j});
            else if (!contains(ObjectBounds::Outer, part))
                if constexpr (Type == config::SolvingMethod::Explicit)
                    t = explicitCentralDifference({i, j});
        }

    if constexpr (Type == config::SolvingMethod::Implicit)
        implicitCentralDifference();

    T(0).swap(T(1));
}