#pragma once
#include <glad/glad.h>

struct Mesh
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    int indexCount = 0;
    GLuint texture = 0;
};
