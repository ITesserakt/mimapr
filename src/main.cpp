#include "Solver.h"
#include <iostream>

int main() {
    auto params = config::TaskParameters::GenerateForVariant(9);
    Mesh m = {params, 5};
    auto s = Solver{std::move(m)};

    std::cout << s.solve();
}