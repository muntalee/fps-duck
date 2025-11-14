#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Mesh
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    int indexCount = 0;
    GLuint texture = 0;
    glm::vec3 color = glm::vec3(1.0f);
};
