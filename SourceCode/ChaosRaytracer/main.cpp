#pragma once

#include <fstream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include "include/rapidjson/document.h"
#include "main.hpp"


// Global variables
const int maxColorComponent = 255;
const int raysPerPixel = 1;
const int numberOfBounces = 8;

int imageWidth = 1920;
int imageHeight = 1080;
Vector3 defaultColor = Vector3(0, 0, 0);
Vector3 cameraPosition = Vector3(0, 0, 3);
Matrix3x3 cameraRotation = Matrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
std::vector<Triangle> triangles;
std::vector<Light> lights;
std::vector<Material> materials;
std::vector<std::vector<Vector3>> imageBuffer;
std::mutex bufferMutex;

void WorldIntersection(const Vector3 ray, const Vector3 position, float& closestDistance, Vector3& surfaceNormal, int& materialIndex)
{
    for (auto& triangle : triangles) {
        Vector3 uv = Vector3(0, 0, 0);
        float distance = triangle.intersect(ray, position, uv);
        if (distance < closestDistance) {
            closestDistance = distance;
            materialIndex = triangle.materialIndex;
            surfaceNormal = triangle.hitNormal(uv.x, uv.y);
        }
    }
}

Vector3 RayTrace(float imageX, float imageY) {
    Vector3 rayOrigin = cameraPosition;
    Vector3 ray = (cameraRotation * Vector3(imageX, imageY, -1)).normalize();
    Vector3 finalColor = Vector3(0, 0, 0);
    Vector3 colorPersistance = Vector3(1, 1, 1);

    for (int bounceNumber = 0; bounceNumber < numberOfBounces; bounceNumber++)
    {
        float distance = std::numeric_limits<float>::infinity();
        Vector3 surfaceNormal = Vector3(0, 0, 0);
        int materialIndex = 0;
        WorldIntersection(ray, rayOrigin, distance, surfaceNormal, materialIndex);
        if (distance == std::numeric_limits<float>::infinity()) {
            finalColor = finalColor + colorPersistance * defaultColor;
            break;
        }

        Vector3 intersectionPoint = rayOrigin + ray * distance;
        Material material = materials[materialIndex];

        colorPersistance = colorPersistance * material.albedo;

        if (material.type == diffuse) {
            Vector3 lightContribution(0, 0, 0);
            for (const auto& light : lights) {
                Vector3 lightDir = (light.position - intersectionPoint).normalize();
                Vector3 fromLightDir = (intersectionPoint - light.position).normalize();

                float closestDistance = std::numeric_limits<float>::infinity();
                Vector3 _dummy1 = Vector3(0, 0, 0);
                int _dummy2 = 0;
                WorldIntersection(fromLightDir, light.position, closestDistance, _dummy1, _dummy2);

                if (closestDistance + EPSILON < (light.position - intersectionPoint).length()) {
                    continue;
                }

                float distanceToLightSq = (light.position - intersectionPoint).lengthSquared();
                float inverseSquareFactor = 1.0f / distanceToLightSq;
                float diffuseIntensity = std::max(0.0f, surfaceNormal.dot(lightDir)) * light.intensity;
                float finalIntensity = diffuseIntensity * inverseSquareFactor;
                lightContribution = lightContribution + Vector3(1, 1, 1) * finalIntensity;
            }
            finalColor = finalColor + colorPersistance * lightContribution;

            break;
        }

        if (material.type == reflective) {
            ray = (ray - surfaceNormal * 2 * (ray.dot(surfaceNormal))).normalize();
            rayOrigin = intersectionPoint - ray * EPSILON;
        }
    }

    return finalColor;
}

int colorFromDecimalToWholeRepresentation(float value) {
    value = std::min(1.0f, std::max(0.0f, value));
    return (int)std::round(value * maxColorComponent);
}

std::string colorToPPMFormat(Vector3 color) {
    return std::to_string(colorFromDecimalToWholeRepresentation(color.x)) + " " + std::to_string(colorFromDecimalToWholeRepresentation(color.y)) + " " + std::to_string(colorFromDecimalToWholeRepresentation(color.z));
}

int rowsCompleted;
void renderFrame(int frameNumber) {
    imageBuffer = std::vector<std::vector<Vector3>>(imageHeight, std::vector<Vector3>(imageWidth, Vector3(0, 0, 0)));
    rowsCompleted = 0;

    auto renderRowRange = [](int startRow, int stepRow) {
        for (int imageY = startRow; imageY < imageHeight; imageY += stepRow) {
            for (int imageX = 0; imageX < imageWidth; ++imageX) {
                Vector3 finalColor = Vector3(0, 0, 0);
                for (int rayNumber = 0; rayNumber < raysPerPixel; rayNumber++) {
                    float randomX = ((float)rand() / (RAND_MAX)) + imageX;
                    float randomY = ((float)rand() / (RAND_MAX)) + imageY;

                    float x = randomX / imageWidth;  // from 0 to 1
                    float y = randomY / imageHeight; // from 0 to 1

                    x = (2.0f * x) - 1.0f; // from -1 to 1
                    y = 1.0f - (2.0f * y); // from -1 to 1

                    float aspectRatio = (float)imageWidth / (float)imageHeight;
                    x *= aspectRatio; // from -ar to ar

                    Vector3 color = RayTrace(x, y);
                    color = Vector3((float)pow(color.x, 2.2), (float)pow(color.y, 2.2), (float)pow(color.z, 2.2)); // gamma correction;
                    finalColor = finalColor + color;
                }
                bufferMutex.lock();
                imageBuffer[imageY][imageX] = finalColor / (float)raysPerPixel;
                bufferMutex.unlock();
            }

            rowsCompleted++;
            std::cout << "progress: " << (float)rowsCompleted / imageHeight * 100.0f << "%\n";
        }
    };

    std::vector<std::thread> threads;
    int maxThreads = std::max(0, (int)std::thread::hardware_concurrency() - 4);

    for (int i = 0; i < maxThreads; ++i) {
        threads.emplace_back(renderRowRange, i, maxThreads);
    }

    for (auto& thread : threads) {
        thread.join();
    }


    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << frameNumber;
    std::string fileName = "output/frame_" + ss.str() + ".ppm";

    std::ofstream ppmFileStream(fileName, std::ios::out | std::ios::binary);
    if (!ppmFileStream.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    // Init ppm
    ppmFileStream << "P3\n" << imageWidth << " " << imageHeight << "\n" << maxColorComponent << "\n";

    for (int imageY = 0; imageY < imageHeight; ++imageY) {
        for (int imageX = 0; imageX < imageWidth; ++imageX) {
            ppmFileStream << colorToPPMFormat(imageBuffer[imageY][imageX]) << " ";
        }
        ppmFileStream << "\n";
    }
    ppmFileStream.close();
}

void loadScene(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Could not open the scene file!" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string sceneContent = buffer.str();
    ifs.close();

    rapidjson::Document document;
    if (document.Parse(sceneContent.c_str()).HasParseError()) {
        std::cerr << "Error parsing the scene file!" << std::endl;
        return;
    }

    if (document.HasMember("settings") && document["settings"].HasMember("background_color")) {
        const auto& backgroundColor = document["settings"]["background_color"].GetArray();
        defaultColor = Vector3(backgroundColor[0].GetFloat(), backgroundColor[1].GetFloat(), backgroundColor[2].GetFloat());
    }

    if (document.HasMember("settings") && document["settings"].HasMember("image_settings")) {
        const auto& imageSettings = document["settings"]["image_settings"];
        if (imageSettings.HasMember("width") && imageSettings.HasMember("height")) {
            imageWidth = imageSettings["width"].GetInt();
            imageHeight = imageSettings["height"].GetInt();
        }
    }

    if (document.HasMember("camera")) {
        const auto& camera = document["camera"];
        if (camera.HasMember("position")) {
            const auto& position = camera["position"].GetArray();
            cameraPosition = Vector3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
        }
        if (camera.HasMember("matrix")) {
            const auto& matrix = camera["matrix"].GetArray();
            cameraRotation = Matrix3x3(
                matrix[0].GetFloat(), matrix[1].GetFloat(), matrix[2].GetFloat(),
                matrix[3].GetFloat(), matrix[4].GetFloat(), matrix[5].GetFloat(),
                matrix[6].GetFloat(), matrix[7].GetFloat(), matrix[8].GetFloat()
            );
        }
    }

    lights.clear();
    if (document.HasMember("lights") && document["lights"].IsArray()) {
        const rapidjson::Value& lightsArray = document["lights"];
        for (rapidjson::SizeType i = 0; i < lightsArray.Size(); ++i) {
            const rapidjson::Value& light = lightsArray[i];
            Vector3 position = Vector3(light["position"][0].GetFloat(), light["position"][1].GetFloat(), light["position"][2].GetFloat());
            float intensity = light["intensity"].GetFloat();
            intensity /= 16;
            lights.push_back(Light(position, intensity));
        }
    }

    materials.clear();
    if (document.HasMember("materials") && document["materials"].IsArray()) {
        const rapidjson::Value& materialsArray = document["materials"];
        for (rapidjson::SizeType i = 0; i < materialsArray.Size(); ++i) {
            const rapidjson::Value& material = materialsArray[i];
            MaterialType materialType = diffuse;
            if (material.HasMember("type") && std::string(material["type"].GetString()) == "diffuse") { materialType = diffuse; }
            if (material.HasMember("type") && std::string(material["type"].GetString()) == "reflective") { materialType = reflective; }
            if (material.HasMember("type") && std::string(material["type"].GetString()) == "refractive") { materialType = refractive; }

            Vector3 albedo(1, 1, 1);
            if (material.HasMember("albedo") && material["albedo"].IsArray()) {
                const auto& albedoArray = material["albedo"].GetArray();
                albedo = Vector3(albedoArray[0].GetFloat(), albedoArray[1].GetFloat(), albedoArray[2].GetFloat());
            }

            bool smoothShading = false;
            if (material.HasMember("smooth_shading")) {
                smoothShading = material["smooth_shading"].GetBool();
            }

            materials.push_back(Material(materialType, albedo, smoothShading));
        }
    }

    triangles.clear();
    if (document.HasMember("objects") && document["objects"].IsArray()) {
        const rapidjson::Value& objects = document["objects"];
        for (rapidjson::SizeType i = 0; i < objects.Size(); ++i) {
            const rapidjson::Value& object = objects[i];
            int materialIndex = -1;
            if (object.HasMember("material_index")) {
                materialIndex = object["material_index"].GetInt();
            }
            if (object.HasMember("vertices") && object["vertices"].IsArray() &&
                object.HasMember("triangles") && object["triangles"].IsArray()) {

                std::vector<Vector3> vertices;
                const rapidjson::Value& verticesArray = object["vertices"];
                for (rapidjson::SizeType j = 0; j < verticesArray.Size(); j += 3) {
                    vertices.push_back(Vector3(verticesArray[j].GetFloat(), verticesArray[j + 1].GetFloat(), verticesArray[j + 2].GetFloat()));
                }

                std::vector<Vector3> vertexNormals(vertices.size(), Vector3(0, 0, 0));

                const rapidjson::Value& trianglesArray = object["triangles"];
                for (rapidjson::SizeType j = 0; j < trianglesArray.Size(); j += 3) {
                    int indexA = trianglesArray[j].GetInt();
                    int indexB = trianglesArray[j + 1].GetInt();
                    int indexC = trianglesArray[j + 2].GetInt();

                    Vector3 vertexA = vertices[indexA];
                    Vector3 vertexB = vertices[indexB];
                    Vector3 vertexC = vertices[indexC];

                    Vector3 edge1 = vertexB - vertexA;
                    Vector3 edge2 = vertexC - vertexA;
                    Vector3 faceNormal = edge1.cross(edge2).normalize();

                    vertexNormals[indexA] = vertexNormals[indexA] + faceNormal;
                    vertexNormals[indexB] = vertexNormals[indexB] + faceNormal;
                    vertexNormals[indexC] = vertexNormals[indexC] + faceNormal;
                }

                for (size_t j = 0; j < vertices.size(); ++j) {
                    vertexNormals[j] = vertexNormals[j].normalize();
                }

                for (rapidjson::SizeType j = 0; j < trianglesArray.Size(); j += 3) {
                    int indexA = trianglesArray[j].GetInt();
                    int indexB = trianglesArray[j + 1].GetInt();
                    int indexC = trianglesArray[j + 2].GetInt();

                    Triangle triangle = Triangle(
                        vertices[indexA],
                        vertices[indexB],
                        vertices[indexC],
                        materialIndex,
                        vertexNormals[indexA],
                        vertexNormals[indexB],
                        vertexNormals[indexC]
                    );

                    if (!materials[materialIndex].smoothShading) {
                        triangle.vertexANormal = triangle.vertexBNormal = triangle.vertexCNormal = triangle.normal();
                    }

                    triangles.push_back(triangle);
                }
            }
        }
    }
}

int main() {
    loadScene("./scenes/11/scene0.crtscene"); renderFrame(0);
    loadScene("./scenes/11/scene1.crtscene"); renderFrame(1);
    loadScene("./scenes/11/scene2.crtscene"); renderFrame(2);
    loadScene("./scenes/11/scene3.crtscene"); renderFrame(3);
    loadScene("./scenes/11/scene4.crtscene"); renderFrame(4);
    loadScene("./scenes/11/scene5.crtscene"); renderFrame(5);
    loadScene("./scenes/11/scene6.crtscene"); renderFrame(5);
    loadScene("./scenes/11/scene7.crtscene"); renderFrame(5);

    return 0;
}
