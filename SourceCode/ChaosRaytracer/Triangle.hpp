#pragma once

#include "Vector3.hpp"

struct Triangle {
	Vector3 vertexA;
	Vector3 vertexB;
	Vector3 vertexC;
	Vector3 vertexANormal;
	Vector3 vertexBNormal;
	Vector3 vertexCNormal;
	int materialIndex;

	Triangle(Vector3 _vertexA, Vector3 _vertexB, Vector3 _vertexC, int _materialIndex, Vector3 _vertexANormal, Vector3 _vertexBNormal, Vector3 _vertexCNormal)
		: vertexA(_vertexA), vertexB(_vertexB), vertexC(_vertexC), materialIndex(_materialIndex), vertexANormal(_vertexANormal), vertexBNormal(_vertexBNormal), vertexCNormal(_vertexCNormal) {}

	[[gnu::pure]]
	Vector3 normal() const {
		Vector3 vectorAB = vertexB - vertexA;
		Vector3 vectorAC = vertexC - vertexA;
		return vectorAB.cross(vectorAC).normalize();
	}

	[[gnu::pure]]
	Vector3 hitNormal(float u, float v) const {
		return (vertexBNormal * u + vertexCNormal * v + vertexANormal * (1 - u - v)).normalize();
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

		// Back face culling
		//if (dotToPlane >= 0) {
		//	return std::numeric_limits<float>::infinity();
		//}

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