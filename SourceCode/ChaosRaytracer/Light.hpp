#pragma once

#include "Vector3.hpp"

struct Light {
	Vector3 position;
	float intensity;

	Light(Vector3 _position, float _intensity)
		: position(_position), intensity(_intensity) {}
};
