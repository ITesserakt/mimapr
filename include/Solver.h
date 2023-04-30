#pragma once

#include <ostream>
#if USE_OPEN_MP
#include <omp.h>
#endif

#include "mesh.h"

template <typename T> using Tensor3 = Eigen::VectorX<Eigen::MatrixX<T>>;

struct Solution {
    Tensor3<Node> timeMesh;
    double step;

    friend std::ostream &operator<<(std::ostream &os, const Solution &solution);

    Solution useOnlyTimeLayer(int layer);
};

enum class Method { Explicit, Implicit };

class Solver {
    using Index = Eigen::Vector2i;

  private:
    Tensor3<Node> T;
    double step;
    config::TaskParameters params;
    int SizeT;
    double dt;

    double explicitCentralDifference(const Index &index);
    double applyBorderConvection(const Index &index);
    double applyBorderInsulation(const Index &index);
    void implicitCentralDifference();

    template <Method Type> void solveNextLayer() {
        using namespace EnumBitmask;

        if constexpr (Type == Method::Implicit) {
            implicitCentralDifference();
            return;
        }

        auto rows = T(0).rows();
        auto cols = T(0).cols();

        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                auto &node = T(1)(i, j);
                if (EnumBitmask::contains(params.border.Heat, node.part) || node.part == ObjectBound::R2) {
                    node.t = 100;
                    if (node.part == ObjectBound::R2)
                        node.t = 200;
                }
                else if (EnumBitmask::contains(params.border.Convection, node.part))
                    node.t = applyBorderConvection({i, j});
                else if (EnumBitmask::contains(params.border.ThermalInsulation, node.part))
                    node.t = applyBorderInsulation({i, j});
                else if (!EnumBitmask::contains(ObjectBounds::Outer, node.part))
                    node.t = explicitCentralDifference({i, j});
            }

        for (int i = 0; i < rows; i++)
            for(int j = 0; j < cols; j++)
                T(0)(i, j).t = T(1)(i, j).t;
    }

  public:
    Solver(Mesh &&mesh, const config::Constants &consts);

    template <Method Type = Method::Explicit> Solution solve() {
#pragma omp parallel for
        for (int currentTime = 0; currentTime < SizeT - 1; currentTime++)
            solveNextLayer<Type>();

        return {T, step};
    }

    [[nodiscard]] Eigen::Vector2d getNormalToBorder(const Index &index,
                                                    const Node &node) const;
};