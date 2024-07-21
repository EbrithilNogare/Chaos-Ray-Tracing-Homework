#pragma once

#include "Scene.hpp"

#include <chrono>

int main() {
    using namespace std::literals;

    auto start = std::chrono::high_resolution_clock::now();
    Scene scene;

    scene.loadScene(SCENES_FOLDER + "/scene0.crtscene"); scene.renderFrame(0);

    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "Done in: " << (stop - start) / 1ms << " ms ~ " << (stop - start) / 1s << " s" << std::endl;
    return 0;
}
