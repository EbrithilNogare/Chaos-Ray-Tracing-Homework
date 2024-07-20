#pragma once

#include "Scene.hpp"

Scene::Scene()
    : defaultColor(Vector3(0, 0, 0)),
    cameraPosition(Vector3(0, 0, 3)),
    cameraRotation(Matrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1)),
    imageWidth(1920),
    imageHeight(1080),
    rowsCompleted(0) {}

void Scene::WorldIntersection(const Vector3 ray, const Vector3 position, float& closestDistance, Vector3& surfaceNormal, int& materialIndex, Vector3& uv, Vector3& interpolatedUV)
{
    for (auto& triangle : triangles) {
        Vector3 currentUV = Vector3(0, 0, 0);
        Vector3 currentInterpolatedUV = Vector3(0, 0, 0);
        float distance = triangle.intersect(ray, position, currentUV, currentInterpolatedUV);
        if (distance < closestDistance) {
            closestDistance = distance;
            materialIndex = triangle.materialIndex;
            surfaceNormal = triangle.hitNormal(uv.x, uv.y);
            uv = currentUV;
            interpolatedUV = currentInterpolatedUV;
        }
    }
}

Vector3 Scene::Refract(const Vector3& incident, const Vector3& normal, float ior)
{
    float cosi = std::max(-1.0f, std::min(1.0f, incident.dot(normal)));
    float etai = 1;
    float etat = ior;
    Vector3 n = normal;

    if (cosi < 0) {
        cosi = -cosi; std::swap(etai, etat);
        n = normal * -1;
    }

    float etaRatio = etai / etat;
    float k = 1 - etaRatio * etaRatio * (1 - cosi * cosi);
    return k < 0 ? Vector3(0, 0, 0) : incident * etaRatio + n * (etaRatio * cosi - sqrtf(k));
}

float Scene::Fresnel(const Vector3& incident, const Vector3& normal, float ior)
{
    float cosi = std::max(-1.0f, std::min(1.0f, incident.dot(normal)));
    float etai = 1;
    float etat = ior;

    if (cosi < 0) {
        cosi = -cosi; std::swap(etai, etat);
    }

    float sint = etai / etat * sqrtf(std::max(0.0f, 1 - cosi * cosi));

    if (sint >= 1) {
        return 1.0f;
    } else {
        float cost = sqrtf(std::max(0.0f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2.0f;
    }
}

Vector3 Scene::Diffuse(Vector3& intersectionPoint, Vector3& surfaceNormal)
{
    Vector3 lightContribution = Vector3(0,0,0);

    for (const auto& light : lights) {
        Vector3 lightDir = (light.position - intersectionPoint).normalize();
        Vector3 fromLightDir = (intersectionPoint - light.position).normalize();

        float closestDistance = std::numeric_limits<float>::infinity();
        Vector3 _dummy1 = Vector3(0, 0, 0);
        int _dummy2 = 0;
        WorldIntersection(fromLightDir, light.position, closestDistance, _dummy1, _dummy2, _dummy1, _dummy1);

        if (closestDistance + EPSILON < (light.position - intersectionPoint).length()) {
            continue;
        }

        float distanceToLightSq = (light.position - intersectionPoint).lengthSquared();
        float inverseSquareFactor = 1.0f / distanceToLightSq;
        float diffuseIntensity = std::max(0.0f, surfaceNormal.dot(lightDir)) * light.intensity;
        float finalIntensity = diffuseIntensity * inverseSquareFactor;
        lightContribution = lightContribution + Vector3(1, 1, 1) * finalIntensity;
    }

    return lightContribution;
}

Vector3 Scene::RayTraceRay(const Vector3& origin, const Vector3& ray, int maxBounces) {
    Vector3 finalColor = Vector3(0, 0, 0);
    Vector3 colorPersistance = Vector3(1, 1, 1);
    Vector3 rayOrigin = origin;
    Vector3 currentRay = ray;

    for (int bounceNumber = 0; bounceNumber < maxBounces; bounceNumber++) {
        float distance = std::numeric_limits<float>::infinity();
        Vector3 surfaceNormal = Vector3(0, 0, 0);
        Vector3 uv = Vector3(0, 0, 0);
        Vector3 interpolatedUV = Vector3(0, 0, 0);
        int materialIndex = 0;
        WorldIntersection(currentRay, rayOrigin, distance, surfaceNormal, materialIndex, uv, interpolatedUV);
        if (distance == std::numeric_limits<float>::infinity()) {
            finalColor = finalColor + colorPersistance * defaultColor;
            break;
        }

        Vector3 intersectionPoint = rayOrigin + currentRay * distance;
        Material& material = materials[materialIndex];

        Vector3 color = material.albedo.GetColorFromMaterial(uv, interpolatedUV);

        colorPersistance = colorPersistance * color;

        if (material.type == diffuse) {
            Vector3 lightContribution = Diffuse(intersectionPoint, surfaceNormal);
            finalColor = finalColor + colorPersistance * lightContribution;
            break;
        }

        if (material.type == reflective) {
            currentRay = (currentRay - surfaceNormal * 2 * (currentRay.dot(surfaceNormal))).normalize();
            rayOrigin = intersectionPoint + currentRay * EPSILON;
        }

        if (material.type == constant) {
            finalColor = finalColor + colorPersistance;
            break;
        }

        if (material.type == refractive) {
            float kr = Fresnel(currentRay, surfaceNormal, material.ior);
            Vector3 reflectedRay = (currentRay - surfaceNormal * 2 * (currentRay.dot(surfaceNormal))).normalize();
            Vector3 refractedRay = Refract(currentRay, surfaceNormal, material.ior).normalize();

            Vector3 reflectedColor = RayTraceRay(intersectionPoint + reflectedRay * EPSILON, reflectedRay, (maxBounces - 1) / 2);
            Vector3 refractedColor = RayTraceRay(intersectionPoint + refractedRay * EPSILON, refractedRay, (maxBounces - 1) / 2);

            finalColor = finalColor + colorPersistance * (reflectedColor * kr + refractedColor * (1 - kr));
            break;
        }

        if (colorPersistance.x < EPSILON && colorPersistance.y < EPSILON && colorPersistance.z < EPSILON) {
            break;
        }
    }

    return finalColor;
}

Vector3 Scene::RayTrace(float imageX, float imageY) {
    Vector3 rayOrigin = cameraPosition;
    Vector3 ray = (cameraRotation * Vector3(imageX, imageY, -1)).normalize();
    return RayTraceRay(rayOrigin, ray, MAXIMUM_RAY_BOUNCES_COUNT);
}


int Scene::colorFromDecimalToWholeRepresentation(float value) {
    value = std::min(1.0f, std::max(0.0f, value));
    return (int)std::round(value * MAX_COLOR_COMPONENT);
}

std::string Scene::colorToPPMFormat(Vector3 color) {
    return std::to_string(colorFromDecimalToWholeRepresentation(color.x)) + " " + std::to_string(colorFromDecimalToWholeRepresentation(color.y)) + " " + std::to_string(colorFromDecimalToWholeRepresentation(color.z));
}

void Scene::renderFrame(int frameNumber) {
    imageBuffer = std::vector<std::vector<Vector3>>(imageHeight, std::vector<Vector3>(imageWidth, Vector3(0, 0, 0)));
    rowsCompleted = 0;

    auto renderRowRange = [this](int startRow, int stepRow) {
        for (int imageY = startRow; imageY < imageHeight; imageY += stepRow) {
            for (int imageX = 0; imageX < imageWidth; ++imageX) {
                Vector3 finalColor = Vector3(0, 0, 0);
                for (int rayNumber = 0; rayNumber < RAYS_PER_PIXEL; rayNumber++) {
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
                imageBuffer[imageY][imageX] = finalColor / (float)RAYS_PER_PIXEL;
                bufferMutex.unlock();
            }

            rowsCompleted++;
            std::cout << std::fixed << std::setprecision(3) << (float)rowsCompleted / imageHeight * 100.0f << "%\n";
        }
    };

    std::vector<std::thread> threads;
    int maxThreads = std::max(1, (int)std::thread::hardware_concurrency() - 1);

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
    ppmFileStream << "P3\n" << imageWidth << " " << imageHeight << "\n" << MAX_COLOR_COMPONENT << "\n";

    for (int imageY = 0; imageY < imageHeight; ++imageY) {
        for (int imageX = 0; imageX < imageWidth; ++imageX) {
            ppmFileStream << colorToPPMFormat(imageBuffer[imageY][imageX]);
            if (imageX < imageWidth - 1) {
                ppmFileStream << " ";
            }
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}

void Scene::loadScene(const std::string& filename) {
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
            intensity *= LIGHT_INTENSITY_CORRECTION;
            lights.push_back(Light(position, intensity));
        }
    }

    textures.clear();
    if (document.HasMember("textures") && document["textures"].IsArray()) {
        const rapidjson::Value& texturesArray = document["textures"];
        for (rapidjson::SizeType i = 0; i < texturesArray.Size(); ++i) {
            const rapidjson::Value& texture = texturesArray[i];
            std::string name = texture["name"].GetString();
            std::string typeStr = texture["type"].GetString();
            TextureType type;

            if (typeStr == "albedo") type = ALBEDO;
            else if (typeStr == "edges") type = EDGES;
            else if (typeStr == "checker") type = CHECKER;
            else if (typeStr == "bitmap") type = BITMAP;
            else throw "Unknown texture type";

            if (type == ALBEDO) {
                const auto& albedoArray = texture["albedo"].GetArray();
                Vector3 albedo = Vector3(albedoArray[0].GetFloat(), albedoArray[1].GetFloat(), albedoArray[2].GetFloat());
                textures.push_back(Texture::CreateAlbedoTexture(name, albedo));
            }
            else if (type == EDGES) {
                const auto& edgeColorArray = texture["edge_color"].GetArray();
                Vector3 edgeColor = Vector3(edgeColorArray[0].GetFloat(), edgeColorArray[1].GetFloat(), edgeColorArray[2].GetFloat());
                const auto& innerColorArray = texture["inner_color"].GetArray();
                Vector3 innerColor = Vector3(innerColorArray[0].GetFloat(), innerColorArray[1].GetFloat(), innerColorArray[2].GetFloat());
                float edgeWidth = texture["edge_width"].GetFloat();
                textures.push_back(Texture::CreateEdgesTexture(name, edgeColor, innerColor, edgeWidth));
            }
            else if (type == CHECKER) {
                const auto& colorAArray = texture["color_A"].GetArray();
                Vector3 colorA = Vector3(colorAArray[0].GetFloat(), colorAArray[1].GetFloat(), colorAArray[2].GetFloat());
                const auto& colorBArray = texture["color_B"].GetArray();
                Vector3 colorB = Vector3(colorBArray[0].GetFloat(), colorBArray[1].GetFloat(), colorBArray[2].GetFloat());
                float squareSize = texture["square_size"].GetFloat();

                textures.push_back(Texture::CreateCheckerTexture(name, colorA, colorB, squareSize));
            }
            else if (type == BITMAP) {
                std::string filePath = texture["file_path"].GetString();
                textures.push_back(Texture::CreateBitmapTexture(name, filePath));
            }
        }
    }

    materials.clear();
    if (document.HasMember("materials") && document["materials"].IsArray()) {
        const rapidjson::Value& materialsArray = document["materials"];
        for (rapidjson::SizeType i = 0; i < materialsArray.Size(); ++i) {
            const rapidjson::Value& material = materialsArray[i];
            
            MaterialType materialType = diffuse;
            if (material.HasMember("type")) {
                if (std::string(material["type"].GetString()) == "diffuse") { materialType = diffuse; }
                if (std::string(material["type"].GetString()) == "reflective") { materialType = reflective; }
                if (std::string(material["type"].GetString()) == "refractive") { materialType = refractive; }
                if (std::string(material["type"].GetString()) == "constant") { materialType = constant; }
            }

            Texture albedo = textures[0];
            if (material.HasMember("albedo")) {
                if (material["albedo"].IsString()) {
                    const auto& albedoName = material["albedo"].GetString();
                    for (auto texture : textures)
                    {
                        if (albedoName == texture.name) {
                            albedo = texture;
                        }
                    }
                }
            }

            float ior = 1.0f;
            if (material.HasMember("ior")) {
                ior = material["ior"].GetFloat();
            }

            bool smoothShading = false;
            if (material.HasMember("smooth_shading")) {
                smoothShading = material["smooth_shading"].GetBool();
            }

            materials.push_back(Material(materialType, albedo, ior, smoothShading));
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
                object.HasMember("uvs") && object["uvs"].IsArray() &&
                object.HasMember("triangles") && object["triangles"].IsArray()) {

                std::vector<Vector3> vertices;
                const rapidjson::Value& verticesArray = object["vertices"];
                for (rapidjson::SizeType j = 0; j < verticesArray.Size(); j += 3) {
                    vertices.push_back(Vector3(verticesArray[j].GetFloat(), verticesArray[j + 1].GetFloat(), verticesArray[j + 2].GetFloat()));
                }

                std::vector<Vector3> vertexUVs;
                const rapidjson::Value& uvArray = object["uvs"];
                for (rapidjson::SizeType j = 0; j < uvArray.Size(); j += 3) {
                    vertexUVs.push_back(Vector3(uvArray[j].GetFloat(), uvArray[j + 1].GetFloat(), uvArray[j + 2].GetFloat()));
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
                        vertexNormals[indexC],
                        vertexUVs[indexA],
                        vertexUVs[indexB],
                        vertexUVs[indexC]
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
