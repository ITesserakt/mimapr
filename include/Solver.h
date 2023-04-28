#pragma once

#include <ostream>
#include <unsupported/Eigen/CXX11/Tensor>

#include "mesh.h"

class Solver {
    using Index = Eigen::Vector2i;

  private:
    Eigen::Tensor<Node, 3> T;
    int currentTime = 0;

    void explicitCentralDifference(const Index &index, int time);

  public:
    explicit Solver(Mesh &&mesh);

    void solveNextLayer();

    friend std::ostream &operator<<(std::ostream &os, const Solver &solver);

    static constexpr int SizeT = 100;
    static constexpr double dt = 0.1;
};
