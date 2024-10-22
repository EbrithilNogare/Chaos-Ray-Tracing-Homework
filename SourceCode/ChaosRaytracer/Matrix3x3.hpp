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

	Vector3 operator*(const Vector3& vec) const {
		return Vector3(
			vec.x * data[0][0] + vec.y * data[1][0] + vec.z * data[2][0],
			vec.x * data[0][1] + vec.y * data[1][1] + vec.z * data[2][1],
			vec.x * data[0][2] + vec.y * data[1][2] + vec.z * data[2][2]
		);
	}

	void setRotationX(float angle) {
		float c = cos(angle);
		float s = sin(angle);
		data[0][0] = 1;  data[0][1] = 0;  data[0][2] =  0;
		data[1][0] = 0;  data[1][1] = c;  data[1][2] = -s;
		data[2][0] = 0;  data[2][1] = s;  data[2][2] =  c;
	}

	void setRotationY(float angle) {
		float c = cos(angle);
		float s = sin(angle);
		data[0][0] =  c; data[0][1] = 0; data[0][2] = s; 
		data[1][0] =  0; data[1][1] = 1; data[1][2] = 0; 
		data[2][0] = -s; data[2][1] = 0; data[2][2] = c; 
	}

	void setRotationZ(float angle) {
		float c = cos(angle);
		float s = sin(angle);
		data[0][0] = c;  data[0][1] = -s; data[0][2] = 0;
		data[1][0] = s;  data[1][1] =  c; data[1][2] = 0;
		data[2][0] = 0;  data[2][1] =  0; data[2][2] = 1;
	}

	void setLookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
		Vector3 forward = (target - eye).normalize();
		Vector3 right = up.cross(forward).normalize();
		Vector3 vertical = forward.cross(right);
		
		data[0][0] = right.x;    data[0][1] = right.y;    data[0][2] = right.z;
		data[1][0] = vertical.x; data[1][1] = vertical.y; data[1][2] = vertical.z;
		data[2][0] = -forward.x; data[2][1] = -forward.y; data[2][2] = -forward.z;
	}
};
