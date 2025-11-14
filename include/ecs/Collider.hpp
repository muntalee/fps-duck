#pragma once
#include <glm/glm.hpp>

struct Collider
{
    enum Type
    {
        AABB,
        Sphere
    } type = AABB;

    // used for AABB
    glm::vec3 halfExtents{0.5f};
    // used for Sphere
    float radius = 0.5f;
};
