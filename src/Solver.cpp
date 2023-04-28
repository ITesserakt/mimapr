#include <iostream>

#include "Solver.h"

Solver::Solver(Mesh &&mesh) {
    const auto &meshMatrix = mesh.getMesh();
    const auto rows = meshMatrix.rows();
    const auto cols = meshMatrix.cols();
    T.resize(rows, cols, SizeT);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            for (int time = 0; time < SizeT; time++)
                T(i, j, time) = meshMatrix(i, j);
}

void Solver::explicitCentralDifference(const Index &index, int time) {
    /**
     *     E
     *     |
     * B - A - C
     *     |
     *     D
     */
    const auto &A = T(index.x(), index.y(), time);
    const auto &B = T(index.x() - 1, index.y(), time);
    const auto &C = T(index.x() + 1, index.y(), time);
    const auto &D = T(index.x(), index.y() - 1, time);
    const auto &E = T(index.x(), index.y() + 1, time);

    double dx = (C.xy - A.xy).norm();
    double dy = (E.xy - A.xy).norm();

    auto &node = T(index.x(), index.y(), time + 1);
    node.t = dt * ((C.t - 2 * A.t + B.t) / dx / dx + (E.t - 2 * A.t + D.t) / dy / dy) + A.t;
}

void Solver::solveNextLayer() {
    auto rows = T.dimension(0);
    auto cols = T.dimension(1);

    for (int i = 1; i < rows - 1; i++)
        for (int j = 1; j < cols - 1; j++) {
            explicitCentralDifference({i, j}, currentTime);
        }
    currentTime++;
}

std::ostream &operator<<(std::ostream &os, const Solver &solver) {
    os << "t x y T" << std::endl;

    auto dims = solver.T.dimensions();

    for (int time = 0; time < dims[2]; time++)
        for (int row = 0; row < dims[0]; row++)
            for (int col = 0; col < dims[1]; col++) {
                const auto &node = solver.T(row, col, time);
                os << time << " " << node.xy.x() << " " << node.xy.y() << " " << node.t << std::endl;
            }

    return os;
}
