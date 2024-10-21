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

    if (consts.Kind == config::RenderKind::RenderGif || consts.Kind == config::RenderKind::OutputAll)
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

    return dt * ((C - 2 * A + B) / dx / dx + (E - 2 * A + D) / dy / dy) + A;
}

double Solver::applyBorderConvection(const Index &index) {
    const auto &node = T(0)(index.x(), index.y());
    Eigen::Vector2d antiNormal = -getNormalToBorder(index, node);
    Eigen::Vector4i indexes = {index.x(), index.y(), index.x(), index.y()};

    /**
     * (C = F) ---- G    |    G ---- (C = F) | (A = E) - (C = B) | (A = C) - (B = E)
     *    |         |    |    |         |    |    |         |    |    |         |
     * (A = D) - (B = E) | (A = E) - (B = D) |    G ---- (D = F) | (D = F) ---- G
     */

    if (antiNormal.y() < 0.)
        indexes.w() = index.y() - 1;
    if (antiNormal.y() >= 0.)
        indexes.y() = index.y() + 1;
    if (antiNormal.x() < 0.)
        indexes.x() = index.x() - 1;
    if (antiNormal.x() >= 0.)
        indexes.z() = index.x() + 1;

    double dx = step, dy = step;
    const auto &A = T(0)(indexes.x(), index.y());
    const auto &B = T(0)(indexes.z(), index.y());
    const auto &C = T(0)(index.x(), indexes.y());
    const auto &D = T(0)(index.x(), indexes.w());

    const auto &E = indexes.x() != index.x() ? A : B; // outstanding by x node
    const auto &F = indexes.y() != index.y() ? C : D; // outstanding by y node
    const auto &G = T(0)(indexes.x() != index.x() ? indexes.x() : indexes.z(),
                         indexes.y() != index.y() ? indexes.y() : indexes.w());

    auto gradX = (A - B) / dx;
    auto gradY = (C - D) / dy;
    auto dxdy = G - E - F + node;

    auto normal = -antiNormal;
    double result = 0.0;
    if (normal.x() != 0)
        result += 1. / normal.x() * (gradX - dxdy * normal.y());
    if (normal.y() != 0)
        result += 1. / normal.y() * (gradY - dxdy * normal.x());
    return result;
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

double Solver::applyBorderInsulation(const Index &index) {
    const auto &node = T(0)(index.x(), index.y());
    Eigen::Vector2d normal = getNormalToBorder(index, node);
    Eigen::Vector2d antiNormal = -normal;
    auto offsetX = antiNormal.x() >= 0. ? -1 : 1;
    auto offsetY = antiNormal.y() >= 0. ? -1 : 1;
    auto i = index.x();
    auto j = index.y();

    double dx = step;
    double dy = step;

    double dxdy = T(0)(i + offsetX, j + offsetY) - T(0)(i + offsetX, j) - T(0)(i, j + offsetY) + T(0)(i, j);

    return dt * (-2 * dxdy / dx / dy) + T(0)(i, j);
}

void Solver::implicitCentralDifference() {
    auto rows = T(0).rows();
    auto cols = T(0).cols();

    Eigen::VectorXd tNew = meshCoeffs.ldlt().solve(meshFreeCoeffs);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            auto &node = T(1)(i, j);
            if (!EnumBitmask::contains(params.border.bound(), node.part) &&
                !EnumBitmask::contains(ObjectBounds::Outer, node.part))
                node.t = tNew(i * cols + j);
        }
}

Eigen::MatrixXd Solver::buildCoefficientMatrix() const {
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

    return coefficients;
}

Solution Solver::solveExplicit() {
    ProgressBar bar{static_cast<float>(SizeT - 1)};
    for (int currentTime = 0; currentTime < SizeT - 1; currentTime++, bar++) {
        if (SavedTemperatures.size() != 1)
            SavedTemperatures(currentTime) = T(0).cast<double>();

        std::cout << bar;
        solveNextLayer<config::SolvingMethod::Explicit>();
    }
    std::cout << "\n";

    if (SavedTemperatures.size() == 1)
        SavedTemperatures(0) = T(0).cast<double>();

    return {std::move(SavedTemperatures), step};
}

Solution Solver::solveImplicit() {
    meshCoeffs = buildCoefficientMatrix();
    meshFreeCoeffs = buildFreeDicksVector();

    ProgressBar bar{static_cast<float>(SizeT - 1)};
    for (int currentTime = 0; currentTime < SizeT - 1; currentTime++, bar++) {
        if (SavedTemperatures.size() != 1)
            SavedTemperatures(currentTime) = T(0).cast<double>();

        std::cout << bar;
        solveNextLayer<config::SolvingMethod::Implicit>();
    }
    std::cout << "\n";

    if (SavedTemperatures.size() == 1)
        SavedTemperatures(0) = T(0).cast<double>();

    return {std::move(SavedTemperatures), step};
}

Eigen::VectorXd Solver::buildFreeDicksVector() const {
    using namespace Eigen;

    auto rows = T(0).rows();
    auto cols = T(0).cols();
    auto dx = step;
    auto dy = step;

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
                    b(i * cols + j) -= dt / dx;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i + 1, j).part))
                    b(i * cols + j) -= dt / dx;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i, j - 1).part))
                    b(i * cols + j) -= dt / dy;
                if (EnumBitmask::contains(params.border.bound(), T(0)(i, j + 1).part))
                    b(i * cols + j) -= dt / dy;
            }
        }

    return b;
}
