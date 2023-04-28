#pragma once

#include <Eigen/Dense>
#include <EnumBitmask.h>
#include <cmath>

enum class NodeType : int {
    INNER = 0,
    OUTER = 1,

    TOP_B = 2,
    BOTTOM_B = 4,
    LEFT_B = 8,
    RIGHT_B = 16,

    SQUARE_B = 32,
    CIRCLE_B = 64,

    NONE = 128,

    SQUARE_IN = 256,
    CIRCLE_IN = 512
};
DEFINE_BITMASK_OPERATORS(NodeType)

struct Node {
    Eigen::Vector2d xy = {0, 0};
    NodeType type = NodeType::NONE;
    double t = 0;

    Node() = default;

    Node(double x, double y, NodeType in_type);
};

class Mesh {
    Eigen::Matrix<Node, Eigen::Dynamic, Eigen::Dynamic> mesh;

  public:
    Mesh(double InX, double InY, double Step);

    [[nodiscard]] const Eigen::Matrix<Node, Eigen::Dynamic, Eigen::Dynamic>& getMesh() const;
};