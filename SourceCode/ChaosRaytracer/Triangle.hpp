#pragma once

#include "Vector3.hpp"
#include "Intersection.hpp"

struct Triangle {
    Vector3 vertexA;
    Vector3 vertexB;
    Vector3 vertexC;
    Vector3 vertexANormal;
    Vector3 vertexBNormal;
    Vector3 vertexCNormal;
    Vector3 vertexAUV;
    Vector3 vertexBUV;
    Vector3 vertexCUV;
    int materialIndex;

    Triangle(Vector3 _vertexA, Vector3 _vertexB, Vector3 _vertexC, int _materialIndex, Vector3 _vertexANormal, Vector3 _vertexBNormal, Vector3 _vertexCNormal, Vector3 _vertexAUV, Vector3 _vertexBUV, Vector3 _vertexCUV)
        : vertexA(_vertexA), vertexB(_vertexB), vertexC(_vertexC), materialIndex(_materialIndex), vertexANormal(_vertexANormal), vertexBNormal(_vertexBNormal), vertexCNormal(_vertexCNormal), vertexAUV(_vertexAUV), vertexBUV(_vertexBUV), vertexCUV(_vertexCUV) {}

    inline Vector3 normal() const {
        Vector3 vectorAB = vertexB - vertexA;
        Vector3 vectorAC = vertexC - vertexA;
        return vectorAB.cross(vectorAC).normalize();
    }

    inline Vector3 hitNormal(float u, float v) const {
        return (vertexBNormal * u + vertexCNormal * v + vertexANormal * (1 - u - v)).normalize();
    }

    float area() const {
        Vector3 vectorAB = vertexB - vertexA;
        Vector3 vectorAC = vertexC - vertexA;
        return 0.5f * vectorAB.cross(vectorAC).length();
    }

    Vector3 centroid() const {
        return (vertexA + vertexB + vertexC) / 3.0f;
    }

    Intersection intersect(Vector3 ray, Vector3 cameraPosition, bool backfaceCullingON) const {
        Intersection intersection = Intersection();

        Vector3 translatedPointA = vertexA - cameraPosition;
        Vector3 translatedPointB = vertexB - cameraPosition;
        Vector3 translatedPointC = vertexC - cameraPosition;

        Vector3 E0 = translatedPointB - translatedPointA;
        Vector3 E1 = translatedPointC - translatedPointB;
        Vector3 E2 = translatedPointA - translatedPointC;
        Vector3 normal = E0.cross(E1);

        float dotOfRayAndNormal = ray.dot(normal);
        if (dotOfRayAndNormal == 0) {
            return intersection;
        }

        float dotToPlane = translatedPointA.dot(normal);
        if (backfaceCullingON && dotToPlane >= 0) {
            return intersection;
        }

        float distance = dotToPlane / dotOfRayAndNormal;
        if (distance < 0) {
            return intersection;
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

        float areaABC = V0V1.cross(V0V2).length();
        intersection.uv.x = C2.length() / areaABC;
        intersection.uv.y = C0.length() / areaABC;
        intersection.uv.z = 1 - intersection.uv.x - intersection.uv.y;

        intersection.interpolatedUV = vertexBUV* intersection.uv.x + vertexCUV * intersection.uv.y + vertexAUV * intersection.uv.z;

        if (normal.dot(C0) > 0 && normal.dot(C1) > 0 && normal.dot(C2) > 0) {
            intersection.distance = distance;
            intersection.type = Hit;
            return intersection;
        }

        return intersection;
    }
};
