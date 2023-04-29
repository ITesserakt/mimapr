#include <cmath>
#include <iostream>

#include "config.h"
#include "mesh.h"

#define X_R2_CENTER (W - R2)
#define Y_R2_CENTER (H - R2)

Mesh::Mesh(const config::TaskParameters &params, double step) {
    this->step = step;
    this->params = params;

    x_count = std::ceil(W / step) + 1;
    y_count = std::ceil(H / step) + 1;

    nodeTypesInit();
    initHeatBorderConditions();
}

void Mesh::nodeTypesInit() {
    nodes.resize(x_count, y_count);

    for (int i = 0; i < x_count; i++)
        for (int j = 0; j < y_count; j++)
            nodes(i, j).part = shape(i * step, j * step);
    nodes(0, 0).part = Outer;

    for (int i = 0; i < x_count; i++) {
        for (int j = 1; j < y_count; j++) {
            auto &left = nodes(i, j - 1).part;
            auto &right = nodes(i, j).part;

            if (left == ObjectBounds::Inner && right == Outer) {
                if (i >= X_R2_CENTER / step)
                    right = ObjectBounds::R2;
                else
                    right = ObjectBounds::T;
            }

            updateOnBorders(left, right);
        }
    }

    for (int i = 1; i < x_count; i++) {
        for (int j = 0; j < y_count; j++) {
            auto &left = nodes(i - 1, j).part;
            auto &right = nodes(i, j).part;

            if (left == ObjectBounds::Inner && right == Outer) {
                if (j >= Y_R2_CENTER / step)
                    right = ObjectBounds::R2;
                else
                    right = ObjectBounds::R;
            }

            updateOnBorders(left, right);
        }
    }
}

void Mesh::updateOnBorders(ObjectBounds &left, ObjectBounds &right) const {
    if (left == ObjectBounds::Inner && right == ObjectBounds::CircleOuter)
        right = ObjectBounds::R1;

    if (left == ObjectBounds::CircleOuter && right == ObjectBounds::Inner)
        left = ObjectBounds::R1;

    if (left == ObjectBounds::Inner && right == ObjectBounds::SquareOuter)
        right = ObjectBounds::S;

    if (left == ObjectBounds::SquareOuter && right == ObjectBounds::Inner)
        left = ObjectBounds::S;
}

ObjectBounds Mesh::shape(double x, double y) {
    // за пределами прямоугольника (справа сверху)
    if ((x >= W) || (y >= H))
        return Outer;

    // левая граница
    if ((x == 0) && (y >= 0) && (y <= H))
        return ObjectBounds::L;

    // нижняя граница
    if ((y == 0) && (x >= 0) && (x <= W))
        return ObjectBounds::B;

    // скругленный правый верхний угол
    double distance_r2 = std::sqrt(std::pow(x - X_R2_CENTER, 2) + std::pow(y - Y_R2_CENTER, 2));

    if ((distance_r2 >= R2) && (x >= X_R2_CENTER) && (y >= Y_R2_CENTER))
        return Outer;

    double centerX = params.hole.center.x();
    double centerY = params.hole.center.y();
    // внутри квадратного отверстия
    if (params.hole.isSquare()) {
        double XSL = centerX - S / 2;
        double XSR = centerX + S / 2;
        double YSB = centerY - S / 2;
        double YST = centerY + S / 2;
        if ((x >= XSL) && (x <= XSR) && (y >= YSB) && (y <= YST))
            return ObjectBounds::SquareOuter;
    }

    // внутри круглого отверстия
    if (params.hole.isCircle()) {
        double distance_r1 = std::sqrt(std::pow(x - centerX, 2) + std::pow(y - centerY, 2));
        if (distance_r1 <= R1)
            return ObjectBounds::CircleOuter;
    }

    return ObjectBounds::Inner;
}

void Mesh::initHeatBorderConditions() {
    for (int i = 0; i < nodes.rows(); i++)
        for (int j = 0; j < nodes.cols(); j++) {
            auto &node = nodes(i, j);
            if ((node.part & params.border.Heat) == node.part)
                node.t = 100;
            if ((node.part & ObjectBounds::R2) == node.part)
                node.t = 200;
        }
}

void Mesh::print_short(ObjectBounds type)
{
    switch (type)
    {
    case ObjectBounds::Inner:
        std::cout << "_";
        break;
    case ObjectBounds::T:
        std::cout << "T";
        break;
    case ObjectBounds::B:
        std::cout << "B";
        break;
    case ObjectBounds::L:
        std::cout << "L";
        break;
    case ObjectBounds::R:
        std::cout << "R";
        break;
    case ObjectBounds::R2:
        std::cout << "2";
        break;
    case ObjectBounds::S:
        std::cout << "3";
        break;
    case ObjectBounds::R1:
        std::cout << "4";
        break;
    case ObjectBounds::CircleOuter:
    case ObjectBounds::SquareOuter:
    case Outer:
        std::cout << "|";
        break ;
    case ObjectBounds::Empty:
        std::cout << "N";
        break;
    }
}

void Mesh::print_node_types()
{
    for (int i = 0; i < x_count; i++)
    {
        for (int j = 0; j < y_count; j++)
        {
            print_short(nodes(i,j).part);
        }
        std::cout << std::endl;
    }
}