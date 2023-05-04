#pragma once

#include "config.h"

struct Node {
    double t = 0.;
    ObjectBound part = ObjectBound::Empty;
    Eigen::Vector2d lambdaMu = {0, 0};

    operator double() const { return t; }
};

class Mesh {
    friend class Solver;

  private:
    Eigen::MatrixX<Node> nodes;

    config::TaskParameters params;

    double step;
    int x_count;
    int y_count;

    double R2;
    double H;
    double W;
    double R1;
    double S;

    double X_R2_CENTER = W - R2;
    double Y_R2_CENTER = H - R2;

    void nodeTypesInit();

    ObjectBound shape(double x, double y);

    void initHeatBorderConditions();

  public:
    Mesh(const config::TaskParameters &params, const config::Constants& consts);

    void print_node_types();
};