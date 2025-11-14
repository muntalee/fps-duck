#pragma once
#include <glm/glm.hpp>

struct Bullet
{
    glm::vec3 dir{0.0f};
    float speed = 30.0f;
    float ttl = 5.0f; // seconds
};
