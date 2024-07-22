#pragma once

#include "Vector3.hpp"

enum IntersectionType {
	Miss,
	Hit
};

struct Intersection {
	int materialIndex;
	float distance;
	IntersectionType type;
	Vector3 uv;
	Vector3 interpolatedUV;
	Vector3 surfaceNormal;

	Intersection() : distance(std::numeric_limits<float>::infinity()), materialIndex(-1), type(Miss) {}
};
