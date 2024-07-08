#pragma once

const float M_PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

[[gnu::pure]]
inline float degreesToRadians(float degrees) {
	return degrees * (M_PI / 180.0f);
}

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z) {}

	[[gnu::pure]]
	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	[[gnu::pure]]
	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	[[gnu::pure]]
	Vector3 operator*(const float& other) const {
		return Vector3(x * other, y * other, z * other);
	}

	[[gnu::pure]]
	Vector3 operator*(const Vector3& other) const {
		return Vector3(x * other.x, y * other.y, z * other.z);
	}

	[[gnu::pure]]
	Vector3 operator/(const float& other) const {
		return Vector3(x / other, y / other, z / other);
	}

	[[gnu::pure]]
	Vector3 normalize() const {
		float len = length();
		if (len == 0 || len == -std::numeric_limits<float>::infinity())
			return Vector3(0, 0, 0); // better than error
		return Vector3(x / len, y / len, z / len);
	}

	[[gnu::pure]]
	Vector3 cross(const Vector3& other) const {
		return Vector3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}

	[[gnu::pure]]
	float dot(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	[[gnu::pure]]
	float length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	[[gnu::pure]]
	float lengthSquared() const {
		return x * x + y * y + z * z;
	}

	[[gnu::pure]]
	float area(const Vector3& other) const {
		return cross(other).length();
	}

	[[gnu::pure]]
	Vector3 rotate(const Vector3& rotation) {
		float pitch = rotation.x * (M_PI / 180.0f);
		float yaw = rotation.y * (M_PI / 180.0f);
		float roll = rotation.z * (M_PI / 180.0f);

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

struct Triangle {
	Vector3 vertexA;
	Vector3 vertexB;
	Vector3 vertexC;
	Vector3 vertexANormal;
	Vector3 vertexBNormal;
	Vector3 vertexCNormal;
	int materialIndex;

	Triangle(Vector3 _vertexA, Vector3 _vertexB, Vector3 _vertexC, int _materialIndex, Vector3 _vertexANormal, Vector3 _vertexBNormal, Vector3 _vertexCNormal)
		: vertexA(_vertexA), vertexB(_vertexB), vertexC(_vertexC), materialIndex(_materialIndex), vertexANormal(_vertexANormal), vertexBNormal(_vertexBNormal), vertexCNormal(_vertexCNormal){}

	[[gnu::pure]]
	Vector3 normal() const {
		Vector3 vectorAB = vertexB - vertexA;
		Vector3 vectorAC = vertexC - vertexA;
		return vectorAB.cross(vectorAC).normalize();
	}

	[[gnu::pure]]
	Vector3 hitNormal(float u, float v) const {
		return (vertexBNormal * u + vertexCNormal * v + vertexANormal * (1-u-v)).normalize();
	}

	[[gnu::pure]]
	float area() const {
		Vector3 vectorAB = vertexB - vertexA;
		Vector3 vectorAC = vertexC - vertexA;
		return 0.5f * vectorAB.cross(vectorAC).length();
	}

	float intersect(Vector3 ray, Vector3 cameraPosition, Vector3& uv) {
		Vector3 translatedPointA = vertexA - cameraPosition;
		Vector3 translatedPointB = vertexB - cameraPosition;
		Vector3 translatedPointC = vertexC - cameraPosition;

		Vector3 E0 = translatedPointB - translatedPointA;
		Vector3 E1 = translatedPointC - translatedPointB;
		Vector3 E2 = translatedPointA - translatedPointC;
		Vector3 normal = E0.cross(E1).normalize();

		float dotOfRayAndNormal = ray.dot(normal);
		if (dotOfRayAndNormal == 0) return std::numeric_limits<float>::infinity();

		float dotToPlane = translatedPointA.dot(normal);

		if (dotToPlane >= 0) {
			return std::numeric_limits<float>::infinity();
		}

		float distance = dotToPlane / dotOfRayAndNormal;
		if (distance < 0) {
			return std::numeric_limits<float>::infinity();
		}
		Vector3 pointOfHit = ray * distance;

		Vector3 V0P = pointOfHit - translatedPointA; // Vector from vertex A to point of hit
		Vector3 V1P = pointOfHit - translatedPointB; // Vector from vertex B to point of hit
		Vector3 V2P = pointOfHit - translatedPointC; // Vector from vertex C to point of hit

		Vector3 V0V2 = translatedPointC - translatedPointA;
		Vector3 V0V1 = translatedPointB - translatedPointA;

		Vector3 C0 = E0.cross(V0P);
		Vector3 C1 = E1.cross(V1P);
		Vector3 C2 = E2.cross(V2P);

		uv.x = V0P.cross(V0V2).length() / V0V1.cross(V0V2).length();
		uv.y = V0V1.cross(V0P).length() / V0V1.cross(V0V2).length();

		if (normal.dot(C0) > 0 && normal.dot(C1) > 0 && normal.dot(C2) > 0) {
			return distance;
		}

		return std::numeric_limits<float>::infinity();
	}
};

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

struct Light {
	Vector3 position;
	float intensity;

	Light(Vector3 _position, float _intensity)
		: position(_position), intensity(_intensity) {}
};

enum MaterialType {
	diffuse,
	reflective,
	refractive
};

struct Material {
	MaterialType type;
	Vector3 albedo;
	bool smoothShading;

	Material(MaterialType _type, Vector3 _albedo, bool _smoothShading) :
		type(_type), albedo(_albedo), smoothShading(_smoothShading) {}
};
