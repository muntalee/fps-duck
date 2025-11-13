#pragma once
#include <glm/glm.hpp>

struct Light
{
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};
