#pragma once

#include <string>
#include <vector>

#include "Vector3.hpp"
#include "Constants.hpp"

enum TextureType {
    ALBEDO,
    EDGES,
    CHECKER,
    BITMAP
};

struct BitmapData {
    int width;
    int height;
    std::vector<Vector3> data;
};

class Texture {
public:
    std::string name;
    TextureType type;
    
    // ALBEDO
    Vector3 albedo; 

    // EDGES
    Vector3 edgeColor;
    Vector3 innerColor;
    float edgeWidth;

    // CHECKER
    Vector3 colorA;
    Vector3 colorB;
    float squareSize;

    // BITMAP
    std::string filePath;
    BitmapData bitmap;

    Texture(std::string _name, TextureType _type) : name(_name), type(_type) {}

    static Texture CreateAlbedoTexture(const std::string& _name, const Vector3& _albedo) {
        Texture tex(_name, ALBEDO);
        tex.albedo = _albedo;
        return tex;
    }

    static Texture CreateEdgesTexture(const std::string& _name, const Vector3& _edgeColor, const Vector3& _innerColor, float _edgeWidth) {
        Texture tex(_name, EDGES);
        tex.edgeColor = _edgeColor;
        tex.innerColor = _innerColor;
        tex.edgeWidth = _edgeWidth;
        return tex;
    }

    static Texture CreateCheckerTexture(const std::string& _name, const Vector3& _colorA, const Vector3& _colorB, float _squareSize) {
        Texture tex(_name, CHECKER);
        tex.colorA = _colorA;
        tex.colorB = _colorB;
        tex.squareSize = _squareSize;
        return tex;
    }

    static Texture CreateBitmapTexture(const std::string& _name, const std::string& _filePath) {
        Texture tex(_name, BITMAP);
        tex.filePath = SCENES_FOLDER + _filePath;
        tex.loadImage();
        return tex;
    }

    Vector3 GetColorFromMaterial(Vector3 uv, Vector3 interpolatedUV) const;

private:
    void loadImage();
};
