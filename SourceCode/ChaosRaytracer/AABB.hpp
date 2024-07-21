#pragma once

#include <vector>

#include "Vector3.hpp"
#include "Triangle.hpp"

struct AABB {
    Vector3 min;
    Vector3 max;
    AABB* parent;
    AABB *childA;
    AABB *childB;
    std::vector<Triangle> triangles;

    AABB(Vector3 _min, Vector3 _max) : min(_min), max(_max), childA(nullptr), childB(nullptr), parent(nullptr) {}

    void expandToInclude(const Vector3& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    bool intersect(const Vector3& rayOrigin, const Vector3& rayDir) const {
        float tmin = (min.x - rayOrigin.x) / rayDir.x;
        float tmax = (max.x - rayOrigin.x) / rayDir.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (min.y - rayOrigin.y) / rayDir.y;
        float tymax = (max.y - rayOrigin.y) / rayDir.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return false;

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (min.z - rayOrigin.z) / rayDir.z;
        float tzmax = (max.z - rayOrigin.z) / rayDir.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return false;

        return true;
    }
};
