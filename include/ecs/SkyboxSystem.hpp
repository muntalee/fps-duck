#pragma once
#include "ecs/System.hpp"
#include "ecs/Shader.hpp"
#include <glad/glad.h>
#include <string>

class SkyboxSystem : public System
{
    GLuint cubemap = 0;
    GLuint vao = 0, vbo = 0;
    Shader *shader = nullptr;
    int faceW = 0;
    int faceH = 0;

public:
    SkyboxSystem();
    ~SkyboxSystem();

    bool LoadFromCrossImage(const std::string &path);

    void Update(Registry &registry, float dt) override;
    void Cleanup();
    bool IsLoaded() const { return cubemap != 0; }

    int GetFaceWidth() const;
    int GetFaceHeight() const;
};
