#include "Solver.h"
#include <iostream>

int main() {
    Mesh m = {500, 500, 1};
    auto s = Solver{std::move(m)};

    for(int i = 0; i < Solver::SizeT - 1; i++)
        s.solveNextLayer();

    std::cout << s;
}