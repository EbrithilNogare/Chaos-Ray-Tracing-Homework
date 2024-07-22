#pragma once

#include <vector>

#include "Vector3.hpp"
#include "Triangle.hpp"

enum Axis {
    AxisX,
    AxisY,
    AxisZ
};

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

    static AABB BuildAccTree(int depth, std::vector<Triangle>& triangles) {
        if (depth > MAX_KDTREE_DEPTH || triangles.size() <= MIN_TRIANGLES_IN_NODE) {
            AABB leafNode = AABB(Vector3(), Vector3());
            leafNode.triangles = triangles;
            for (const auto& triangle : triangles) {
                leafNode.expandToInclude(triangle.vertexA);
                leafNode.expandToInclude(triangle.vertexB);
                leafNode.expandToInclude(triangle.vertexC);
            }
            return leafNode;
        }


        Axis axis = static_cast<Axis>(depth % 3);
        std::nth_element(triangles.begin(), triangles.begin() + triangles.size() / 2, triangles.end(), [axis](const Triangle& a, const Triangle& b) {
            switch (axis) {
                case AxisX: return a.centroid().x < b.centroid().x;
                case AxisY: return a.centroid().y < b.centroid().y;
                case AxisZ: return a.centroid().z < b.centroid().z;
            }
        });


        std::vector<Triangle> leftTriangles(triangles.begin(), triangles.begin() + triangles.size() / 2);
        std::vector<Triangle> rightTriangles(triangles.begin() + triangles.size() / 2, triangles.end());

        AABB node = AABB(Vector3(), Vector3());
        node.childA = new AABB(BuildAccTree(depth + 1, leftTriangles));
        node.childB = new AABB(BuildAccTree(depth + 1, rightTriangles));
        node.expandToInclude(node.childA->min);
        node.expandToInclude(node.childA->max);
        node.expandToInclude(node.childB->min);
        node.expandToInclude(node.childB->max);

        return node;
    }

    bool isLeaf() const {
        return childA == nullptr && childB == nullptr;
    }

};
