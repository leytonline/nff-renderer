#include "Engine.h"
#include "Controller.h"
#include "CudaRaytracer.h"
#include "NaiveRaytracer.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
template <typename RendererT>
double benchRenderer(RendererT& renderer, uint32_t* pixels, Controller& controller, int warmupFrames, int timedFrames) {
    for (int i = 0; i < warmupFrames; i++) {
        renderer.Render(pixels, controller.GetPosition(), controller.GetOrientation());
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < timedFrames; i++) {
        renderer.Render(pixels, controller.GetPosition(), controller.GetOrientation());
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / timedFrames;
}

uint32_t packBenchColor(const Eigen::Vector3d& color) {
    uint32_t r = static_cast<uint32_t>(std::min(1.0, std::max(0.0, color[0])) * 255.0);
    uint32_t g = static_cast<uint32_t>(std::min(1.0, std::max(0.0, color[1])) * 255.0);
    uint32_t b = static_cast<uint32_t>(std::min(1.0, std::max(0.0, color[2])) * 255.0);
    return 0xff000000u | (r << 16) | (g << 8) | b;
}

void printPixelStats(const std::vector<uint32_t>& pixels, uint32_t bg) {
    size_t bgCount = 0;
    size_t blackCount = 0;

    for (uint32_t pixel : pixels) {
        bgCount += pixel == bg;
        blackCount += pixel == 0;
    }

    std::cout << "Pixels: bg=" << bgCount
              << ", black=" << blackCount
              << ", total=" << pixels.size()
              << ", bgColor=0x" << std::hex << std::setw(6) << std::setfill('0') << bg
              << std::dec << std::setfill(' ') << std::endl;
}

bool writePpm(const std::string& path, const std::vector<uint32_t>& pixels, int width, int height) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    out << "P6\n" << width << " " << height << "\n255\n";
    for (uint32_t pixel : pixels) {
        char rgb[3] = {
            static_cast<char>((pixel >> 16) & 0xff),
            static_cast<char>((pixel >> 8) & 0xff),
            static_cast<char>(pixel & 0xff)
        };
        out.write(rgb, sizeof(rgb));
    }

    return true;
}

int runBench(int argc, char** argv) {
    std::string scenePath = "scenes/balls-2.nff";
    std::string ppmPath;
    bool cudaOnly = false;

    for (int i = 2; i < argc; i++) {
        if (std::strcmp(argv[i], "--cuda-only") == 0) {
            cudaOnly = true;
        } else if (std::strcmp(argv[i], "--ppm") == 0 && i + 1 < argc) {
            ppmPath = argv[++i];
        } else {
            scenePath = argv[i];
        }
    }

    Nff scene;
    if (scene.parse(scenePath) < 0) {
        std::cerr << "Failed to parse nff image" << std::endl;
        return 1;
    }

    Controller controller;
    controller.InitializeView(scene.GetFrom(), scene.GetUp(), scene.GetAt());

    std::vector<uint32_t> pixels(scene._res.first * scene._res.second);
    std::cout << "Scene: " << scenePath << " ("
              << scene._res.first << "x" << scene._res.second << ", "
              << scene._surfaces.size() << " surfaces, "
              << scene._analyticSpheres.size() << " analytic spheres, "
              << scene._lights.size() << " lights)" << std::endl;

    CudaRaytracer cudaRenderer;
    cudaRenderer.SetNff(&scene);
    double cudaMs = benchRenderer(cudaRenderer, pixels.data(), controller, 3, 20);
    std::cout << "CudaRaytracer: " << cudaMs << " ms/frame (" << 1000.0 / cudaMs << " FPS)" << std::endl;
    printPixelStats(pixels, packBenchColor(scene._bg));
    if (!ppmPath.empty()) {
        if (!writePpm(ppmPath, pixels, scene._res.first, scene._res.second)) {
            std::cerr << "Failed to write " << ppmPath << std::endl;
            return 1;
        }
        std::cout << "Wrote " << ppmPath << std::endl;
    }

    if (cudaOnly) return 0;

    NaiveRaytracer cpuRenderer;
    cpuRenderer.SetNff(&scene);
    double cpuMs = benchRenderer(cpuRenderer, pixels.data(), controller, 1, 5);
    std::cout << "NaiveRaytracer: " << cpuMs << " ms/frame (" << 1000.0 / cpuMs << " FPS)" << std::endl;
    printPixelStats(pixels, packBenchColor(scene._bg));

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "--bench") == 0) {
        return runBench(argc, argv);
    }

    Engine e;
    if (e.MainLoop() != 0)
    {
        std::cout << "ended badly" << std::endl;
    }
    else
    {
        std::cout << "ended gracefully" << std::endl;
    }

   
}
