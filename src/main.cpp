// ReSharper disable once CppUnusedIncludeDirective
#include <heatmap.h>

#include <colorschemes/Spectral.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

#if USE_OPEN_MP
#include <omp.h>
#endif

#include "Solver.h"
#include "drawer.h"
#include "ffmpeg.h"
#include "mesh.h"

void process_solution(const config::Constants &constants, const Solution &solution) {
    if (constants.Kind == config::RenderKind::RenderLast) {
        auto writer = ImageWriter({constants.Width, constants.Height});
        for (int i = 0; i < solution.timeMesh(0).rows(); i++)
            for (int j = 0; j < solution.timeMesh(0).cols(); j++) {
                auto weight = (float)solution.timeMesh(0)(i, j);
                writer.addPoint(i * solution.step, constants.Height - j * solution.step, std::abs(weight));
            }
        std::cerr << "Heatmap populated, generating image" << std::endl;
        std::ofstream output{"image.png", std::ios::out | std::ios::binary};
        output << writer.write(heatmap_cs_Spectral_mixed);
    } else if (constants.Kind == config::RenderKind::OutputLast) {
        std::cout << "t x y T" << std::endl;
        for (int i = 0; i < solution.timeMesh(0).rows(); i++)
            for (int j = 0; j < solution.timeMesh(0).cols(); j++)
                std::cout << constants.TimeLayers - 1 << " " << i * solution.step << " " << j * solution.step << " "
                          << solution.timeMesh(0)(i, j) << std::endl;
    } else if (constants.Kind == config::RenderKind::OutputAll) {
        std::cout << "t x y T" << std::endl;
        for (int time = 0; time < constants.TimeLayers - 1; time++)
            for (int i = 0; i < solution.timeMesh(0).rows(); i++)
                for (int j = 0; j < solution.timeMesh(0).cols(); j++)
                    std::cout << time << " " << i * solution.step << " " << j * solution.step << " "
                              << solution.timeMesh(time)(i, j) << std::endl;
    } else if (constants.Kind == config::RenderKind::RenderGif) {
        auto gifWriter = GifImageWriter{heatmap_cs_Spectral_soft};
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
    } else if (constants.Kind == config::RenderKind::RenderVideo) {
        std::size_t fps = 240;
        auto ffmpeg = FFMPEG{constants.Width, constants.Height, fps};

        for (int time = 0; time < constants.TimeLayers - 1; time++) {
            auto frameWriter = ImageWriter{{constants.Width, constants.Height}};
            for (int i = 0; i < solution.timeMesh(0).rows(); i++)
                for (int j = 0; j < solution.timeMesh(0).cols(); j++) {
                    auto weight = (float)solution.timeMesh(time)(i, j);
                    frameWriter.addPoint(i * solution.step, constants.Height - j * solution.step, std::abs(weight));
                }
            auto handle = frameWriter.write(heatmap_cs_Spectral_soft);
            ffmpeg.send_frame(reinterpret_cast<const std::uint32_t *>(handle.Data));
        }
    }
}

int main() {
    auto current_path = std::filesystem::current_path();
    auto config_path = current_path.parent_path().append("res/config.yml");

    auto yaml = YAML::LoadFile(config_path.string());
    auto constants = yaml["constants"].as<config::Constants>();
    if (!constants.isDefault())
        std::cerr << "Using non-default constant values:\n" << yaml["constants"] << std::endl;

#ifdef USE_OPEN_MP
    omp_set_num_threads(constants.Parallelism);
#endif

    auto params = config::TaskParameters::GenerateForVariant(constants.Variant - 1);
    auto mesh = Mesh{params, constants};
    if (constants.ExportMeshOnly) {
        std::ofstream{"mesh.png", std::ios::out | std::ios::binary} << mesh.exportMesh();
        return 0;
    }

    auto solver = Solver{std::move(mesh), constants};
    std::cerr << "Mesh created. Solving linear systems..." << std::endl;

    Solution solution;
    if (constants.SolveMethod == config::SolvingMethod::Explicit)
        solution = solver.solveExplicit();
    else
        solution = solver.solveImplicit();
    std::cerr << "Successfully calculated solution" << std::endl;

    process_solution(constants, solution);
}
