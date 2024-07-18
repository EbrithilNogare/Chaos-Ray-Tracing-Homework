#pragma once

#include "Vector3.hpp"

enum MaterialType {
	diffuse,
	reflective,
	refractive,
	constant
};

struct Material {
	MaterialType type;
	Vector3 albedo;
	float ior;
	bool smoothShading;

	Material(MaterialType _type, Vector3 _albedo, float _ior, bool _smoothShading) :
		type(_type), albedo(_albedo), smoothShading(_smoothShading), ior(_ior) {}
};
