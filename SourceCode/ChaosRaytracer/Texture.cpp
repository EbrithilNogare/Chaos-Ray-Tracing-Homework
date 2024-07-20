#pragma once

#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include "stb_image.h"

void Texture::loadImage() {
    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 3);
    if (!data) {
        throw std::runtime_error("Failed to load image: " + filePath);
    }

    bitmap.width = width;
    bitmap.height = height;
    bitmap.data.resize(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            bitmap.data[y * width + x] = Vector3(
                data[index] / 255.0f,
                data[index + 1] / 255.0f,
                data[index + 2] / 255.0f
            );
        }
    }

    stbi_image_free(data);
}

Vector3 Texture::GetColorFromMaterial(Vector3 uv, Vector3 interpolatedUV) const {
    switch (type) {
    case ALBEDO:
        return albedo;
    case EDGES: {
        float distanceToEdge = std::min(std::min(uv.x, uv.y), 1-uv.x-uv.y);
        return distanceToEdge < edgeWidth ? edgeColor : innerColor;
    }
    case CHECKER: {
        int checkX = (int)(std::floor(interpolatedUV.x / squareSize)) % 2;
        int checkY = (int)(std::floor(interpolatedUV.y / squareSize)) % 2;
        return (checkX == checkY) ? colorA : colorB;
    }
    case BITMAP: {
        int x = std::clamp((int)(interpolatedUV.x * bitmap.width),0, bitmap.width);
        int y = std::clamp((int)((1.0f - interpolatedUV.y) * bitmap.height), 0, bitmap.height);
        return bitmap.data[y * bitmap.width + x];
    }
    default:
        return Vector3(0, 0, 0);
    }
}
