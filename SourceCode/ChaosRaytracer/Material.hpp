#pragma once

#include "Vector3.hpp"
#include "Texture.hpp"

enum MaterialType {
	diffuse,
	reflective,
	refractive,
	constant
};

struct Material {
	MaterialType type;
	Texture albedo;
	float ior;
	bool smoothShading;

	Material(MaterialType _type, Texture _albedo, float _ior, bool _smoothShading) :
		type(_type), albedo(_albedo), smoothShading(_smoothShading), ior(_ior) {}
};
