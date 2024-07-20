#pragma once

#include "Scene.hpp"

int main() {
    Scene scene;
    scene.loadScene(SCENES_FOLDER + "/scene0.crtscene"); scene.renderFrame(0);
    scene.loadScene(SCENES_FOLDER + "/scene1.crtscene"); scene.renderFrame(1);
    scene.loadScene(SCENES_FOLDER + "/scene2.crtscene"); scene.renderFrame(2);
    scene.loadScene(SCENES_FOLDER + "/scene3.crtscene"); scene.renderFrame(3);
    scene.loadScene(SCENES_FOLDER + "/scene4.crtscene"); scene.renderFrame(4);

    return 0;
}
