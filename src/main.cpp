#include <iostream>

#include <config.h>

int main() {
    int var;
    std::cin >> var;

    const auto foo = config::TaskParameters::GenerateForVariant(var);
}