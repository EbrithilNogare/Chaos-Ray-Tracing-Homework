#pragma once

#include <chrono>

#include "Scene.hpp"

int main() {
    using namespace std::literals;

    auto start = std::chrono::high_resolution_clock::now();
    Scene scene;



    /**/
    scene.loadScene(SCENES_FOLDER + "/scene1.crtscene"); scene.renderFrame(1);
    /**/



    /*/
    scene.loadScene(SCENES_FOLDER + "/scene2.crtscene");
    float distance = 4.0f;
    size_t totalFrames = 24 * 4;
    float angleIncrement = 2 * M_PI / totalFrames;
    Vector3 centralPoint = Vector3(0,0,-3);
    for (size_t i = 52; i >= 0; i--)
    {
        float angle = i * angleIncrement;
        scene.cameraPosition = centralPoint + Vector3(distance * cos(angle), 0, distance * sin(angle));
        scene.cameraRotation.setLookAt(scene.cameraPosition, centralPoint, Vector3(0, 1, 0));
        scene.renderFrame(i);
    }
    /**/


    
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "Done in: " << (stop - start) / 1ms << " ms ~ " << (stop - start) / 1s << " s" << std::endl;
    return 0;
}
