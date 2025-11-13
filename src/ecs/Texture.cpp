#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ecs/Texture.hpp"
#include <iostream>

GLuint Texture::Load(const std::string &path)
{
    int width, height, channels;
    // stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load image: " << path << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return tex;
}

void Texture::Free(GLuint id)
{
    if (id)
        glDeleteTextures(1, &id);
}
