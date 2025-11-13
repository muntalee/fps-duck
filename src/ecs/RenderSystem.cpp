#include "ecs/RenderSystem.hpp"
#include <glm/glm.hpp>
#include <iostream>

static const char *vShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
out vec2 vTex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main() {
    vTex = aTex;
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}
)";

static const char *fShader = R"(
#version 330 core
in vec2 vTex;
out vec4 FragColor;
uniform sampler2D tex0;
void main() {
    FragColor = texture(tex0, vTex);
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
        if (mesh->texture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh->texture);
            GLint loc = glGetUniformLocation(shader->id, "tex0");
            if (loc >= 0)
                glUniform1i(loc, 0);
        }
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void RenderSystem::Cleanup()
{
    delete shader;
}
