#include "ecs/Shader.hpp"
#include <iostream>

static GLuint Compile(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error:\n"
                  << info << std::endl;
    }
    return shader;
}

Shader::Shader(const char *vertexSrc, const char *fragmentSrc)
{
    GLuint vs = Compile(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = Compile(GL_FRAGMENT_SHADER, fragmentSrc);

    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    GLint linked;
    glGetProgramiv(id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char info[512];
        glGetProgramInfoLog(id, 512, nullptr, info);
        std::cerr << "Program link error:\n"
                  << info << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() { glDeleteProgram(id); }

void Shader::SetMat4(const std::string &name, const float *value) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, value);
}
