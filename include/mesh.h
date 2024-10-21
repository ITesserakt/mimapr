#pragma once

#include "config.h"
#include "drawer.h"

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

    const double X_R2_CENTER = W - R2;
    const double Y_R2_CENTER = H - R2;

    void nodeTypesInit();

    ObjectBound shape(double x, double y);

    void initHeatBorderConditions();

    Eigen::Vector2d fixComplexBorders(int x, int y);

  public:
    Mesh(config::TaskParameters params, const config::Constants& consts);

    [[nodiscard]] ImageHandle exportMesh() const;
};