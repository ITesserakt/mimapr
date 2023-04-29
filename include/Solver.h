#pragma once

#include <ostream>
#include <unsupported/Eigen/CXX11/Tensor>

#include "mesh.h"

struct Solution {
    Eigen::Tensor<Node, 3> timeMesh;
    double step;

    friend std::ostream &operator<<(std::ostream &os, const Solution &solution);
};

enum class Method { Explicit, Implicit };

class Solver {
    using Index = Eigen::Vector2i;

  private:
    Eigen::Tensor<Node, 3> T;
    double step;
    config::TaskParameters params;

    void explicitCentralDifference(const Index &index, int time);
    void implicitCentralDifference(int time);
    void applyBorderConvection(const Index &index, int time);

    template <Method Type> void solveNextLayer(int time) {
        if constexpr (Type == Method::Implicit) {
            implicitCentralDifference(time);
            return;
        }

        auto rows = T.dimension(0);
        auto cols = T.dimension(1);

        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                const auto &node = T(i, j, time);
                if ((node.part & params.border.Convection) == node.part)
                    applyBorderConvection({i, j}, time);
                else if ((node.part & params.border.Heat) == node.part || (node.part & ObjectBounds::R2) == node.part)
                    continue;
                else if ((node.part & Outer) != node.part)
                    explicitCentralDifference({i, j}, time);
            }
    }

  public:
    explicit Solver(Mesh &&mesh);

    template <Method Type = Method::Explicit> Solution solve() {
        for (int currentTime = 1; currentTime < SizeT - 1; currentTime++)
            solveNextLayer<Type>(currentTime);

        return {T, step};
    }

    static constexpr int SizeT = 1000;
    static constexpr double dt = 0.9;
    [[nodiscard]] Eigen::Vector2d getNormalToBorder(const Index &index, const Node &node) const;
};