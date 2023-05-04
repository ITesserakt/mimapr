#include <Eigen/Eigenvalues>
#include <iostream>

#include "Solver.h"

Solver::Solver(Mesh &&mesh, const config::Constants &consts)
    : step(mesh.step), params(mesh.params), SizeT(consts.TimeLayers),
      dt(consts.DeltaTime) {
    const auto &meshMatrix = mesh.nodes;
    const auto rows = meshMatrix.rows();
    const auto cols = meshMatrix.cols();
    T.resize(2);

    for (int time = 0; time < 2; time++) {
        T(time).resize(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                auto &node = T(time)(i, j);
                auto &meshNode = meshMatrix(i, j);
                node.part = meshNode.part;
                node.lambdaMu = meshNode.lambdaMu;
                if (time == 0)
                    node.t = meshNode.t;
            }
    }

    if (consts.RenderKind == config::RenderKind::RenderGif || consts.RenderKind == config::RenderKind::OutputAll)
        SavedTemperatures.resize(consts.TimeLayers);
    else
        SavedTemperatures.resize(1);
}

double Solver::explicitCentralDifference(const Index &index) {
    /**
     *     E
     *     |
     * B - A - C
     *     |
     *     D
     */
    const auto &A = T(0)(index.x(), index.y());
    const auto &B = T(0)(index.x() - 1, index.y());
    const auto &C = T(0)(index.x() + 1, index.y());
    const auto &D = T(0)(index.x(), index.y() - 1);
    const auto &E = T(0)(index.x(), index.y() + 1);

    double dx = step;
    double dy = step;

    return dt * ((C.t - 2 * A.t + B.t) / dx / dx +
                 (E.t - 2 * A.t + D.t) / dy / dy) +
           A.t;
}

double Solver::applyBorderConvection(const Index &index) {
    const auto &node = T(0)(index.x(), index.y());
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
    auto gradX =
        (T(0)(indexes.x(), index.y()).t - T(0)(indexes.z(), index.y()).t) / dx;
    auto gradY =
        (T(0)(index.x(), indexes.y()).t - T(0)(index.x(), indexes.w()).t) / dy;

    Eigen::Vector2d gradient = {gradX, gradY};

    return gradient.dot(Eigen::Vector2d{-antiNormal.x(), antiNormal.y()});
}

Eigen::Vector2d Solver::getNormalToBorder(const Solver::Index &index, const Node &node) const {
    Eigen::Vector2d normal;

    if (node.part == ObjectBound::L)
        normal = {-1, 0};
    if (node.part == ObjectBound::R)
        normal = {1, 0};
    if (node.part == ObjectBound::T)
        normal = {0, 1};
    if (node.part == ObjectBound::B)
        normal = {0, -1};
    if (node.part == ObjectBound::R2)
        normal =
            -Eigen::Vector2d{350. - index.x() * step, 250. - index.y() * step};
    if (node.part == ObjectBound::R1)
        normal = (params.hole.center - Eigen::Vector2d{index.x() * step, index.y() * step});
    if (node.part == ObjectBound::S) {
        Eigen::Vector2d vec = (params.hole.center - Eigen::Vector2d{index.x() * step, index.y() * step});
        normal = vec.x() > vec.y() ? Eigen::Vector2d{0, vec.y()} : Eigen::Vector2d{vec.x(), 0};
    }

    return normal.normalized();
}

double Solver::applyBorderInsulation(const Index &index) { return T(0)(index.x(), index.y()).t; }

void Solver::implicitCentralDifference() {
    using namespace Eigen;

    MatrixXd coefficients = MatrixXd::Zero(T(0).size(), T(0).size());
    auto dx = step;
    auto dy = step;

    coefficients.diagonal().setConstant(1. + 2. * dt / dx + 2. * dt / dy);
    coefficients.diagonal(1).setConstant(dt / dy);
    coefficients.diagonal(2).setConstant(dt / dx);
    coefficients.diagonal(-1).setConstant(dt / dy);
    coefficients.diagonal(-2).setConstant(dt / dx);

    auto rows = T(0).rows();
    auto cols = T(0).cols();

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            const auto &node = T(0)(i, j);
            if (EnumBitmask::contains(params.border.bound(), node.part)) {
                coefficients.row(i * cols + j).setZero();
                coefficients.col(i * cols + j).setZero();
            }
        }

    VectorXd b = VectorXd::Zero(T(0).size());
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            const auto &node = T(0)(i, j);
            if (EnumBitmask::contains(params.border.bound(), node.part) ||
                EnumBitmask::contains(ObjectBounds::Outer, node.part))
                b(i * cols + j) = 0;
            else {
                b(i * cols + j) = node.t;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i - 1, j).part))
                    b(i * cols + j) += dt / dx;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i + 1, j).part))
                    b(i * cols + j) += dt / dx;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i, j - 1).part))
                    b(i * cols + j) += dt / dy;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i, j + 1).part))
                    b(i * cols + j) += dt / dy;
            }
        }

    coefficients.ldlt().solveInPlace(b);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            auto &node = T(1)(i, j);
            if (!EnumBitmask::contains(params.border.bound(), node.part))
                node.t = b(i * cols + j);
        }
}
