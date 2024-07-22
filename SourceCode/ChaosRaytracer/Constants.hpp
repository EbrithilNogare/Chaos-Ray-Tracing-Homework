#pragma once

#include <string>
#include <mutex>

// Settings
const int RAYS_PER_PIXEL = 8;
const int MAXIMUM_RAY_BOUNCES_COUNT = 6;
const float LIGHT_INTENSITY_CORRECTION = 1 / 8.0f / 3.0f;
const std::string SCENES_FOLDER = "./scenes/15";
const int THREADS_TO_USE = std::max(1, (int)std::thread::hardware_concurrency() - 1);
const int MAX_KDTREE_DEPTH = 16;
const int MIN_TRIANGLES_IN_NODE = 4;

// Constants
const int MAX_COLOR_COMPONENT = 255;
const float M_PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;
