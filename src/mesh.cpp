#include <cmath>
#include <iostream>

#include "EnumBitmask.h"
#include "config.h"
#include "mesh.h"

Mesh::Mesh(const config::TaskParameters &params,
           const config::Constants &consts)
    : H(consts.Height), W(consts.Width), R1(consts.Radius1), R2(consts.Radius2),
      S(consts.SquareSide) {
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

void Mesh::nodeTypesInit() {
    nodes.resize(x_count, y_count);

    for (int i = 0; i < x_count; i++)
        for (int j = 0; j < y_count; j++)
            nodes(i, j).part = shape(i * step, j * step);
    nodes(0, 0).part = ObjectBounds::Outer;

    for (int i = 0; i < x_count; i++) {
        for (int j = 1; j < y_count; j++) {
            auto &left = nodes(i, j - 1).part;
            auto &right = nodes(i, j).part;

            if (left == ObjectBound::Inner && right == ObjectBounds::Outer) {
                if (i >= X_R2_CENTER / step)
                    right = ObjectBound::R2;
                else
                    right = ObjectBound::T;
            }

            updateOnBorders(left, right);
        }
    }

    for (int i = 1; i < x_count; i++) {
        for (int j = 0; j < y_count; j++) {
            auto &left = nodes(i - 1, j).part;
            auto &right = nodes(i, j).part;

            if (left == ObjectBound::Inner && right == ObjectBounds::Outer) {
                if (j >= Y_R2_CENTER / step)
                    right = ObjectBound::R2;
                else
                    right = ObjectBound::R;
            }

            updateOnBorders(left, right);
        }
    }
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

void Mesh::print_node_types() {
    for (int i = 0; i < x_count; i++) {
        for (int j = 0; j < y_count; j++)
            print_short(nodes(i, j).part);

        std::cout << std::endl;
    }
}