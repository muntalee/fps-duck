#pragma once
#include <string>
#include <glad/glad.h>

class Shader
{
public:
    GLuint id = 0;
    Shader(const char *vertexSrc, const char *fragmentSrc);
    void Use() const { glUseProgram(id); }
    void SetMat4(const std::string &name, const float *value) const;
    ~Shader();
};
