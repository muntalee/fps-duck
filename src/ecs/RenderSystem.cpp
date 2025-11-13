#include "ecs/RenderSystem.hpp"
#include <glm/glm.hpp>
#include <iostream>

static const char *vShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main() {
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}
)";

static const char *fShader = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.2, 0.7, 1.0, 1.0);
}
)";

RenderSystem::RenderSystem()
{
    shader = new Shader(vShader, fShader);

    view = glm::lookAt(glm::vec3(3, 2, 6),
                       glm::vec3(0, 0, 0),
                       glm::vec3(0, 1, 0));

    proj = glm::perspective(glm::radians(45.0f),
                            800.0f / 600.0f, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
}

void RenderSystem::Update(Registry &registry, float dt)
{
    if (!shader || shader->id == 0)
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Use();

    // update camera view matrix from any Camera component, then upload
    for (auto [e, cam] : registry.View<Camera>())
    {
        view = cam->GetViewMatrix();
        break;
    }

    shader->SetMat4("view", &view[0][0]);
    shader->SetMat4("proj", &proj[0][0]);

    for (auto [e, transform] : registry.View<Transform>())
    {
        auto mesh = registry.GetComponent<Mesh>(e);
        if (!mesh)
            continue;

        shader->SetMat4("model", &transform->GetMatrix()[0][0]);
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void RenderSystem::Cleanup()
{
    delete shader;
}
