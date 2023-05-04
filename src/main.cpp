#define _SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING // NOLINT(bugprone-reserved-identifier)

#include <heatmap.h>
#include <colorschemes/Spectral.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

#if USE_OPEN_MP
#include <omp.h>
#include <thread>
#endif

#include "Solver.h"
#include "drawer.h"
#include "mesh.h"

void process_solution(const config::Constants &constants, const Solution &solution) {
    auto writer = ImageWriter({constants.Width, constants.Height});
    auto gifWriter = GifImageWriter{heatmap_cs_Spectral_soft};

    switch (constants.RenderKind) {
    case config::RenderKind::RenderLast:
        for (int i = 0; i < solution.timeMesh(0).rows(); i++)
            for (int j = 0; j < solution.timeMesh(0).cols(); j++) {
                auto weight = (float) solution.timeMesh(0)(i, j);
                writer.addPoint(i * solution.step, constants.Height - j * solution.step, std::abs(weight));
            }
        std::cerr << "Heatmap populated, generating image" << std::endl;
        {
            std::ofstream output{"image.png", std::ios::out | std::ios::binary};
            output << writer.write(heatmap_cs_Spectral_mixed);
        }
        break;
    case config::RenderKind::OutputLast:
        std::cout << "t x y T" << std::endl;
        for (int i = 0; i < solution.timeMesh(0).rows(); i++)
            for (int j = 0; j < solution.timeMesh(0).cols(); j++)
                std::cout << constants.TimeLayers - 1 << " " << i * solution.step << " " << j * solution.step << " "
                          << solution.timeMesh(0)(i, j) << std::endl;
        break;
    case config::RenderKind::OutputAll:
        std::cout << "t x y T" << std::endl;
        for (int time = 0; time < constants.TimeLayers; time++)
            for (int i = 0; i < solution.timeMesh(0).rows(); i++)
                for (int j = 0; j < solution.timeMesh(0).cols(); j++)
                    std::cout << time << " " << i * solution.step << " " << j * solution.step << " "
                              << solution.timeMesh(time)(i, j) << std::endl;
        break;
    case config::RenderKind::RenderGif:
        for (int time = 0; time < constants.TimeLayers - 1; time++) {
            auto frameWriter = ImageWriter{{constants.Width, constants.Height}};
            for (int i = 0; i < solution.timeMesh(0).rows(); i++)
                for (int j = 0; j < solution.timeMesh(0).cols(); j++) {
                    auto weight = (float)solution.timeMesh(time)(i, j);
                    frameWriter.addPoint(i * solution.step, constants.Height - j * solution.step, std::abs(weight));
                }
            gifWriter.addFrame(std::move(frameWriter));
        }
        std::cerr << "Heatmaps populated, generating image" << std::endl;
        std::filesystem::remove("image.gif");
        gifWriter.saveToFile("image.gif");
        break;
    }
}

int main() {
    auto current_path = std::filesystem::current_path();
    auto config_path = current_path.parent_path().append("res/config.yml");

    auto yaml = YAML::LoadFile(config_path.string());
    auto constants = yaml["constants"].as<config::Constants>();
    if (!constants.isDefault())
        std::cerr << "Using non-default constant values:\n" << yaml["constants"] << std::endl;

    auto params = config::TaskParameters::GenerateForVariant(constants.Variant - 1);
    auto mesh = Mesh{params, constants};
    auto solver = Solver{std::move(mesh), constants};
    std::cerr << "Mesh created. Solving linear systems..." << std::endl;

    Solution solution;
    if (constants.SolveMethod == config::SolvingMethod::Explicit)
        solution = solver.solve<config::SolvingMethod::Explicit>();
    else
        solution = solver.solve<config::SolvingMethod::Implicit>();
    std::cerr << "Successfully calculated solution" << std::endl;

    process_solution(constants, solution);
}