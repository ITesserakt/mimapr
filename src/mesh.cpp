#include <array>
#include <cmath>
#include <iostream>

#include "EnumBitmask.h"
#include "config.h"
#include "mesh.h"

Mesh::Mesh(const config::TaskParameters &params, const config::Constants &consts)
    : R2(consts.Radius2), H(consts.Height), W(consts.Width), R1(consts.Radius1), S(consts.SquareSide) {
    this->step = consts.GridStep;
    this->params = params;

    x_count = (int)std::ceil(W / step) + 1;
    y_count = (int)std::ceil(H / step) + 1;

    nodeTypesInit();
    initHeatBorderConditions();
}

static void updateOnBorders(ObjectBound &left, ObjectBound &right) {
    using enum ObjectBound;

    if (left == Inner && right == CircleOuter)
        right = R1;

    if (left == CircleOuter && right == Inner)
        left = R1;

    if (left == Inner && right == SquareOuter)
        right = S;

    if (left == SquareOuter && right == Inner)
        left = S;
}

using Segment = Eigen::MatrixX2d;

static Eigen::Vector2d distanceToSegment(const Eigen::Vector2d &point, Segment segment) {
    const Eigen::Vector2d &head = segment.row(0);
    const Eigen::Vector2d &tail = segment.row(1);
    auto lengthSqr = (head - tail).squaredNorm();
    if (lengthSqr == 0.0)
        return (head - point).norm() * (head - point).normalized().cwiseAbs();

    double t = std::max(0.0, std::min(1.0, (point - head).dot(tail - head) / lengthSqr));
    auto projection = head + t * (tail - head);

    return (point - projection).norm() * (point - projection).normalized().cwiseAbs();
}

static Eigen::Vector2d distanceToSquare(const Eigen::Vector2d &point, double size, Eigen::Vector2d center) {
    return {std::min({distanceToSegment(point, Segment{{center.x() - size / 2., center.y() - size / 2.},
                                                       {center.x() + size / 2., center.y() - size / 2.}})
                          .x(),
                      distanceToSegment(point, Segment{{center.x() + size / 2., center.y() + size / 2.},
                                                       {center.x() + size / 2., center.y() - size / 2.}})
                          .x()}),
            std::min({distanceToSegment(point, Segment{{center.x() - size / 2., center.y() - size / 2.},
                                                       {center.x() - size / 2., center.y() + size / 2.}})
                          .y(),
                      distanceToSegment(point, Segment{{center.x() + size / 2., center.y() + size / 2.},
                                                       {center.x() - size / 2., center.y() + size / 2.}})
                          .y()})};
}

static Eigen::Vector2d distanceToCircle(const Eigen::Vector2d &point, double radius, const Eigen::Vector2d &center) {
    auto link = point - center;
    auto direction = (link.norm() - radius) * link.normalized();
    return direction;
}

static Eigen::Vector2d distanceToArc(const Eigen::Vector2d &point, double radius, Eigen::Vector2d center, double step) {
    if (point.x() > center.x() && point.y() > center.y())
        return distanceToCircle(point, radius, center);

    return {step, step};
}

Eigen::Vector2d Mesh::fixComplexBorders(int x, int y) {
    Eigen::Vector2d point = {x * step, y * step};

    Eigen::Vector2d distLeft = distanceToSegment(point, Segment{{0., 0.}, {0., H}}) / step;
    Eigen::Vector2d distBottom = distanceToSegment(point, Segment{{0., 0.}, {W, 0.}}) / step;
    Eigen::Vector2d distRight = distanceToSegment(point, Segment{{W, 0.}, {W, H - R2}}) / step;
    Eigen::Vector2d distTop = distanceToSegment(point, Segment{{0., H}, {W - R2, H}}) / step;
    Eigen::Vector2d distHole = (params.hole.isSquare() ? distanceToSquare(point, S, params.hole.center)
                                                       : distanceToCircle(point, R1, params.hole.center)) /
                               step;
    Eigen::Vector2d distRadius = distanceToArc(point, R2, {X_R2_CENTER, Y_R2_CENTER}, step) / step;

    Eigen::Vector2d minimumDist = {1 / sqrt(2), 1 / sqrt(2)};
    for (const auto &vec : {distRadius, distHole, distLeft, distRight, distTop, distBottom}) {
        if (vec.norm() < 1.) {
            if (vec.x() < minimumDist.x())
                minimumDist.x() = vec.x();
            if (vec.y() < minimumDist.y())
                minimumDist.y() = vec.y();
        }
    }
    return minimumDist;
}

void Mesh::nodeTypesInit() {
    nodes.resize(x_count, y_count);

    for (int i = 0; i < x_count; i++)
        for (int j = 0; j < y_count; j++)
            nodes(i, j).part = shape(i * step, j * step);
    nodes(0, 0).part = ObjectBounds::Outer;

    for (int i = 0; i < x_count; i++) {
        for (int j = 1; j < y_count; j++) {
            auto &left = nodes(i, j - 1);
            auto &right = nodes(i, j);

            if (left.part == ObjectBound::Inner && right.part == ObjectBounds::Outer) {
                if (i >= X_R2_CENTER / step)
                    right.part = ObjectBound::R2;
                else
                    right.part = ObjectBound::T;
            }

            updateOnBorders(left.part, right.part);
        }
    }

    for (int i = 1; i < x_count; i++) {
        for (int j = 0; j < y_count; j++) {
            auto &left = nodes(i - 1, j);
            auto &right = nodes(i, j);

            if (left.part == ObjectBound::Inner && right.part == ObjectBounds::Outer) {
                if (j >= Y_R2_CENTER / step)
                    right.part = ObjectBound::R2;
                else
                    right.part = ObjectBound::R;
            }

            updateOnBorders(left.part, right.part);
        }
    }

    for (int i = 0; i < x_count; i++)
        for (int j = 0; j < y_count; j++)
            nodes(i, j).lambdaMu = fixComplexBorders(i, j);
}

ObjectBound Mesh::shape(double x, double y) {
    // за пределами прямоугольника (справа сверху)
    if ((x >= W) || (y >= H))
        return ObjectBounds::Outer;

    // левая граница
    if ((x == 0) && (y >= 0) && (y <= H))
        return ObjectBound::L;

    // нижняя граница
    if ((y == 0) && (x >= 0) && (x <= W))
        return ObjectBound::B;

    // скругленный правый верхний угол
    double distance_r2 =
        std::sqrt(std::pow(x - X_R2_CENTER, 2) + std::pow(y - Y_R2_CENTER, 2));

    if ((distance_r2 >= R2) && (x >= X_R2_CENTER) && (y >= Y_R2_CENTER))
        return ObjectBounds::Outer;

    double centerX = params.hole.center.x();
    double centerY = params.hole.center.y();
    // внутри квадратного отверстия
    if (params.hole.isSquare()) {
        double XSL = centerX - S / 2;
        double XSR = centerX + S / 2;
        double YSB = centerY - S / 2;
        double YST = centerY + S / 2;
        if ((x >= XSL) && (x <= XSR) && (y >= YSB) && (y <= YST))
            return ObjectBound::SquareOuter;
    }

    // внутри круглого отверстия
    if (params.hole.isCircle()) {
        double distance_r1 =
            std::sqrt(std::pow(x - centerX, 2) + std::pow(y - centerY, 2));
        if (distance_r1 <= R1)
            return ObjectBound::CircleOuter;
    }

    return ObjectBound::Inner;
}

void Mesh::initHeatBorderConditions() {
    for (int i = 0; i < nodes.rows(); i++)
        for (int j = 0; j < nodes.cols(); j++) {
            auto &node = nodes(i, j);
            if (EnumBitmask::contains(params.border.Heat, node.part))
                node.t = 100;
            if (node.part == ObjectBound::R2)
                node.t = 200;
        }
}

ImageHandle Mesh::exportMesh() const {
    ImageWriter w{{x_count, y_count}};
    heatmap_stamp_t *cell = heatmap_stamp_gen(0);

    for (int x = 0; x < x_count; x++)
        for (int y = 0; y < y_count; y++) {
            const auto &node = nodes(x, y);
            w.addPoint(x, y, Eigen::Vector3d{node.lambdaMu.x(), node.lambdaMu.y(), 0}.norm(), cell);
        }

    heatmap_stamp_free(cell);
    return w.write();
}

static void print_short(ObjectBound type) {
    switch (type) {
    case ObjectBound::Inner:
        std::cout << "_";
        break;
    case ObjectBound::T:
        std::cout << "T";
        break;
    case ObjectBound::B:
        std::cout << "B";
        break;
    case ObjectBound::L:
        std::cout << "L";
        break;
    case ObjectBound::R:
        std::cout << "R";
        break;
    case ObjectBound::R2:
        std::cout << "2";
        break;
    case ObjectBound::S:
        std::cout << "3";
        break;
    case ObjectBound::R1:
        std::cout << "4";
        break;
    case ObjectBound::CircleOuter:
    case ObjectBound::SquareOuter:
    case ObjectBounds::Outer:
        std::cout << "|";
        break;
    case ObjectBound::Empty:
        std::cout << "N";
        break;
    }
}