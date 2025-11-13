#pragma once
#include <string>
#include <glad/glad.h>

class Texture
{
public:
    // returns 0 on failure.
    static GLuint Load(const std::string &path);
    static void Free(GLuint id);
};
