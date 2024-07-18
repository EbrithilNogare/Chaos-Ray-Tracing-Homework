#pragma once

#include <fstream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include <cmath>
#include <limits>

#include "include/rapidjson/document.h"

#include "Vector3.hpp"
#include "Matrix3x3.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include "Material.hpp"
#include "Constants.hpp"

class Scene {
public:
    Scene();

    void loadScene(const std::string& filename);
    void renderFrame(int frameNumber);

private:
    Vector3 defaultColor;
    Vector3 cameraPosition;
    Matrix3x3 cameraRotation;
    std::vector<Triangle> triangles;
    std::vector<Light> lights;
    std::vector<Material> materials;
    std::vector<std::vector<Vector3>> imageBuffer;
    std::mutex bufferMutex;
    int imageWidth;
    int imageHeight;
    int rowsCompleted;

    void WorldIntersection(const Vector3 ray, const Vector3 position, float& closestDistance, Vector3& surfaceNormal, int& materialIndex);
    Vector3 Refract(const Vector3& incident, const Vector3& normal, float eta);
    float Fresnel(const Vector3& incident, const Vector3& normal, float ior);
    Vector3 Diffuse(Vector3& intersectionPoint, Vector3& surfaceNormal);
    Vector3 RayTrace(float imageX, float imageY);
    Vector3 RayTraceRay(const Vector3& origin, const Vector3& ray, int maxBounces);
    int colorFromDecimalToWholeRepresentation(float value);
    std::string colorToPPMFormat(Vector3 color);
};
