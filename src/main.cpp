#define _SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING // NOLINT(bugprone-reserved-identifier)

#include <filesystem>
#include <iostream>
#include <yaml-cpp/yaml.h>

#if USE_OPEN_MP
#include <omp.h>
#include <thread>
#endif

#include "Solver.h"
#include "mesh.h"

int main() {
    auto current_path = std::filesystem::current_path();
    auto config_path = current_path.append("resources/config.yml");

    auto yaml = YAML::LoadFile(config_path.string());
    auto constants = yaml["constants"].as<config::Constants>();

    auto params = config::TaskParameters::GenerateForVariant(constants.Variant);
    Mesh m = {params, constants};
    auto s = Solver{std::move(m), constants};

    if (!constants.OnlyLastTimeLayer)
        std::cout << s.solve();
    else
        std::cout << s.solve().useOnlyTimeLayer(1);
}