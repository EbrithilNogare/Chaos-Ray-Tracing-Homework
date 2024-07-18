#pragma once

#include "Vector3.hpp"

struct Matrix3x3 {
	float data[3][3];

	Matrix3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22) {
		data[0][0] = m00;
		data[0][1] = m01;
		data[0][2] = m02;
		data[1][0] = m10;
		data[1][1] = m11;
		data[1][2] = m12;
		data[2][0] = m20;
		data[2][1] = m21;
		data[2][2] = m22;
	}

	[[gnu::pure]]
	Vector3 operator*(const Vector3& vec) const {
		return Vector3(
			vec.x * data[0][0] + vec.y * data[1][0] + vec.z * data[2][0],
			vec.x * data[0][1] + vec.y * data[1][1] + vec.z * data[2][1],
			vec.x * data[0][2] + vec.y * data[1][2] + vec.z * data[2][2]
		);
	}
};