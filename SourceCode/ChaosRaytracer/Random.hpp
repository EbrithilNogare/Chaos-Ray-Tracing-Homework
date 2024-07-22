#include <random>
#include "Vector3.hpp"

float RandomFloat() {
    static std::mt19937 generator(std::random_device{}());
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    return distribution(generator);
}

Vector3 RandomHemisphereDirection(const Vector3& normal) {
    float u1 = RandomFloat();
    float u2 = RandomFloat();

    float r = sqrt(1.0f - u1 * u1);
    float phi = 2 * M_PI * u2;

    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = u1;

    Vector3 tangent, bitangent;

    if (fabs(normal.x) > fabs(normal.y)) {
        tangent = Vector3(-normal.z, 0, normal.x).normalize();
    }
    else {
        tangent = Vector3(0, normal.z, -normal.y).normalize();
    }

    bitangent = normal.cross(tangent);

    return tangent * x + bitangent * y + normal * z;
}
