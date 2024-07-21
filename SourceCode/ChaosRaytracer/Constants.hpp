#pragma once

#include <string>
#include <mutex>

// Settings
const int RAYS_PER_PIXEL = 1;
const int MAXIMUM_RAY_BOUNCES_COUNT = 16;
const float LIGHT_INTENSITY_CORRECTION = 1 / 8.0f;
const std::string SCENES_FOLDER = "./scenes/13";
const int THREADS_TO_USE = std::max(1, (int)std::thread::hardware_concurrency() - 1);

// Constants
const int MAX_COLOR_COMPONENT = 255;
const float M_PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;
