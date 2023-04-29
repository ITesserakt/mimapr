#pragma once

#include <Eigen/Dense>

#include "config.h"

struct Node {
    double t = 0.;
    ObjectBounds part = ObjectBounds::Empty;
    Eigen::Vector2d lambdaMu = {0, 0};
};

class Mesh {
    friend class Solver;

  private:
    Eigen::Matrix<Node, Eigen::Dynamic, Eigen::Dynamic> nodes;

    config::TaskParameters params;

    double step;
    int x_count;
    int y_count;

    void nodeTypesInit();

    ObjectBounds shape(double x, double y);

    void updateOnBorders(ObjectBounds &left, ObjectBounds &right) const;

    void initHeatBorderConditions();

  public:
    Mesh(const config::TaskParameters &params, double step);

    static constexpr double R2 = 150;
    static constexpr double H = 400;
    static constexpr double W = 500;
    static constexpr double R1 = 50;
    static constexpr double S = 100;
    void print_node_types();
    void print_short(ObjectBounds type);
};