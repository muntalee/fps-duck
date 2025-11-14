#pragma once
#include "ecs/System.hpp"
#include "ecs/Shader.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Mesh.hpp"
#include "ecs/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
class SkyboxSystem;

class RenderSystem : public System
{
    Shader *shader;
    glm::mat4 view, proj;

public:
    RenderSystem();
    void Update(Registry &registry, float dt) override;
    void SetSkybox(SkyboxSystem *s) { skybox = s; }
    void Cleanup();

private:
    SkyboxSystem *skybox = nullptr;
};
