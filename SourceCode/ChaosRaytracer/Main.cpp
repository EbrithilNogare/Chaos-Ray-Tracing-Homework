#include "Scene.hpp"

int main() {
    Scene scene;
    scene.loadScene("./scenes/11/scene0.crtscene"); scene.renderFrame(0);
    scene.loadScene("./scenes/11/scene1.crtscene"); scene.renderFrame(1);
    scene.loadScene("./scenes/11/scene2.crtscene"); scene.renderFrame(2);
    scene.loadScene("./scenes/11/scene3.crtscene"); scene.renderFrame(3);
    scene.loadScene("./scenes/11/scene4.crtscene"); scene.renderFrame(4);
    scene.loadScene("./scenes/11/scene5.crtscene"); scene.renderFrame(5);
    scene.loadScene("./scenes/11/scene6.crtscene"); scene.renderFrame(6);
    scene.loadScene("./scenes/11/scene7.crtscene"); scene.renderFrame(7);
    scene.loadScene("./scenes/11/scene8.crtscene"); scene.renderFrame(8);

    return 0;
}
