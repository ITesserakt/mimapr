#include "mesh.h"

Node::Node(double x, double y, NodeType in_type) {
    xy = {x, y};
    type = in_type;
}

Mesh::Mesh(double InX, double InY, double Step) {
    int XCount = std::ceil(InX / Step);
    int YCount = std::ceil(InY / Step);
    mesh.resize(XCount, YCount);

    for (int i = 0; i < XCount; i++) {
        for (int j = 0; j < YCount; j++) {
            mesh(i, j).xy = Eigen::Vector2d{i, j} * Step;
        }
    }

    for (int i = 0; i < XCount; i++) {
        mesh(i, 0).t = 0;
        mesh(i, YCount - 1).t = 40;
    }

    for (int i = 0; i < YCount; i++) {
        mesh(0, i).t = i * 3.7;
        mesh(XCount - 1, i).t = i * 3.5;
    }
}

const Eigen::Matrix<Node, Eigen::Dynamic, Eigen::Dynamic> &Mesh::getMesh() const { return mesh; }
