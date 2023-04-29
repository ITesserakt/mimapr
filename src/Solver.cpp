#include <iostream>

#include "Solver.h"

Solver::Solver(Mesh &&mesh) : step(mesh.step), params(mesh.params) {
    const auto &meshMatrix = mesh.nodes;
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

    double dx = step;
    double dy = step;

    auto &node = T(index.x(), index.y(), time + 1);
    node.t = dt * ((C.t - 2 * A.t + B.t) / dx / dx + (E.t - 2 * A.t + D.t) / dy / dy) + A.t;
}

void Solver::applyBorderConvection(const Index &index, int time) {
    auto &node = T(index.x(), index.y(), time);
    Eigen::Vector2d antiNormal = -getNormalToBorder(index, node);
    Eigen::Vector4i indexes = {index.x(), index.y(), index.x(), index.y()};

    if (antiNormal.y() < 0.)
        indexes.w() = index.y() - 1;
    if (antiNormal.y() >= 0.)
        indexes.y() = index.y() + 1;
    if (antiNormal.x() < 0.)
        indexes.x() = index.x() - 1;
    if (antiNormal.x() >= 0.)
        indexes.z() = index.x() + 1;

    double dx = step, dy = step;
    Eigen::Vector2d gradient = {
        (T(indexes.x(), index.y(), time).t - T(indexes.z(), index.y(), time).t) / dx,
        (T(index.x(), indexes.y(), time).t - T(index.x(), indexes.w(), time).t) / dy
    };

    node.t = gradient.dot(-antiNormal);
}

Eigen::Vector2d Solver::getNormalToBorder(const Solver::Index &index, const Node &node) const {
    Eigen::Vector2d normal;

    if (node.part == ObjectBounds::L)
        normal = {-1, 0};
    if (node.part == ObjectBounds::R)
        normal = {1, 0};
    if (node.part == ObjectBounds::T)
        normal = {0, 1};
    if (node.part == ObjectBounds::B)
        normal = {0, -1};
    if (node.part == ObjectBounds::R2)
        normal = -Eigen::Vector2d{350 - index.x() * step, 250 - index.y() * step}.normalized();
    if (node.part == ObjectBounds::R1)
        normal = (params.hole.center - Eigen::Vector2d{index.x() * step, index.y() * step}).normalized();
    if (node.part == ObjectBounds::S) {
        Eigen::Vector2d vec = (params.hole.center - Eigen::Vector2d{index.x() * step, index.y() * step});
        normal = vec.x() > vec.y() ? Eigen::Vector2d{0, vec.y()} : Eigen::Vector2d{vec.x(), 0};
        normal.normalize();
    }

    return normal;
}

std::ostream &operator<<(std::ostream &os, const Solution &solution) {
    auto dims = solution.timeMesh.dimensions();

    for (int time = 0; time < dims[2]; time++)
        for (int row = 0; row < dims[0]; row++)
            for (int col = 0; col < dims[1]; col++) {
                const auto &node = solution.timeMesh(row, col, time);
                os << time << " " << row * solution.step << " " << col * solution.step << " " << node.t << std::endl;
            }

    return os;
}
