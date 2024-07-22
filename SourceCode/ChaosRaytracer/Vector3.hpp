#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>

#include "Constants.hpp"

struct Vector3 {
	union
	{
		struct { float x, y, z; };
		struct { float u, v, s; };
		struct { float r, g, b; };
	};


	Vector3() : x(0), y(0), z(0) {}

	Vector3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z) {}


	inline Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }

	inline Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }

	inline Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }

	inline Vector3 operator/(const Vector3& other) const { return Vector3(x / other.x, y / other.y, z / other.z); }

	inline Vector3 operator*(const float& other) const { return Vector3(x * other, y * other, z * other); }

	inline Vector3 operator/(const float& other) const { return Vector3(x / other, y / other, z / other); }


	inline float dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
	
	inline float length() const { return std::sqrt(x * x + y * y + z * z); }
	
	inline float lengthSquared() const { return x * x + y * y + z * z; }
	
	inline float area(const Vector3& other) const { return cross(other).length(); }

	inline Vector3 cross(const Vector3& other) const { return Vector3( y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
	

	Vector3 normalize() const {
		float len = length();
		if (len == 0)
			return Vector3(0, 0, 0); // better than error
		return Vector3(x / len, y / len, z / len);
	}

	Vector3 rotate(const Vector3& rotation) {
		const float degToRad = M_PI / 180.0f;
		float pitch = rotation.x * degToRad;
		float yaw = rotation.y * degToRad;
		float roll = rotation.z * degToRad;

		float cosPitch = cos(pitch);
		float sinPitch = sin(pitch);
		float cosYaw = cos(yaw);
		float sinYaw = sin(yaw);
		float cosRoll = cos(roll);
		float sinRoll = sin(roll);

		float m00 = cosYaw * cosRoll;
		float m01 = -cosYaw * sinRoll;
		float m02 = sinYaw;
		float m10 = sinPitch * sinYaw * cosRoll + cosPitch * sinRoll;
		float m11 = -sinPitch * sinYaw * sinRoll + cosPitch * cosRoll;
		float m12 = -sinPitch * cosYaw;
		float m20 = -cosPitch * sinYaw * cosRoll + sinPitch * sinRoll;
		float m21 = cosPitch * sinYaw * sinRoll + sinPitch * cosRoll;
		float m22 = cosPitch * cosYaw;

		return Vector3(
			x * m00 + y * m01 + z * m02,
			x * m10 + y * m11 + z * m12,
			x * m20 + y * m21 + z * m22
		);
	}
};
