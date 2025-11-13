#include "ecs/RenderSystem.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include "ecs/Light.hpp"
#include "ecs/RenderSystem.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include "ecs/Light.hpp"
#include <fstream>
#include <sstream>
#include <string>

// load file contents into a string
static std::string ReadFileToString(const std::string &path)
{
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in)
        return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

RenderSystem::RenderSystem()
{
    // load shaders from data/shaders/*
    std::string vs = ReadFileToString("data/shaders/vertex.glsl");
    std::string fs = ReadFileToString("data/shaders/fragment.glsl");
    const char *vsrc = nullptr;
    const char *fsrc = nullptr;
    if (!vs.empty() && !fs.empty())
    {
        vsrc = vs.c_str();
        fsrc = fs.c_str();
    }
    else
    {
        std::cerr << "Warning: failed to load shaders from data/shaders/" << std::endl;
    }

    shader = new Shader(vsrc, fsrc);

    view = glm::lookAt(glm::vec3(3, 2, 6),
                       glm::vec3(0, 0, 0),
                       glm::vec3(0, 1, 0));

    proj = glm::perspective(glm::radians(45.0f),
                            800.0f / 600.0f, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_FRAMEBUFFER_SRGB);
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

    // find first light in scene (if any)
    glm::vec3 lightPos(0.0f);
    glm::vec3 lightColor(1.0f);
    float lightIntensity = 1.0f;
    for (auto [le, lptr] : registry.View<Light>())
    {
        auto ltransform = registry.GetComponent<Transform>(le);
        if (ltransform && lptr)
        {
            lightPos = ltransform->position;
            lightColor = lptr->color;
            lightIntensity = lptr->intensity;
            break;
        }
    }

    // find camera position for specular/view calculations
    glm::vec3 viewPos(0.0f);
    for (auto [ce, cam] : registry.View<Camera>())
    {
        viewPos = cam->pos;
        break;
    }

    // set global light uniforms
    GLint locLP = glGetUniformLocation(shader->id, "lightPos");
    if (locLP >= 0)
        glUniform3f(locLP, lightPos.x, lightPos.y, lightPos.z);
    GLint locLC = glGetUniformLocation(shader->id, "lightColor");
    if (locLC >= 0)
        glUniform3f(locLC, lightColor.x, lightColor.y, lightColor.z);
    GLint locLI = glGetUniformLocation(shader->id, "lightIntensity");
    if (locLI >= 0)
        glUniform1f(locLI, lightIntensity);
    GLint locVP = glGetUniformLocation(shader->id, "viewPos");
    if (locVP >= 0)
        glUniform3f(locVP, viewPos.x, viewPos.y, viewPos.z);

    for (auto [e, transform] : registry.View<Transform>())
    {
        auto mesh = registry.GetComponent<Mesh>(e);
        if (!mesh)
            continue;

        shader->SetMat4("model", &transform->GetMatrix()[0][0]);
        glBindVertexArray(mesh->vao);
        GLint useTexLoc = glGetUniformLocation(shader->id, "useTex");
        GLint objColorLoc = glGetUniformLocation(shader->id, "objectColor");

        if (mesh->texture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh->texture);
            GLint loc = glGetUniformLocation(shader->id, "tex0");
            if (loc >= 0)
                glUniform1i(loc, 0);
            if (useTexLoc >= 0)
                glUniform1i(useTexLoc, 1);
            // ensure non-emitter by default
            GLint isEmitterLoc = glGetUniformLocation(shader->id, "isEmitter");
            if (isEmitterLoc >= 0)
                glUniform1i(isEmitterLoc, 0);
        }
        else
        {
            if (useTexLoc >= 0)
                glUniform1i(useTexLoc, 0);
            auto lightComp = registry.GetComponent<Light>(e);
            if (lightComp)
            {
                if (objColorLoc >= 0)
                    glUniform3f(objColorLoc, lightComp->color.r, lightComp->color.g, lightComp->color.b);
            }
            else
            {
                if (objColorLoc >= 0)
                    glUniform3f(objColorLoc, 1.0f, 1.0f, 1.0f);
            }
            // if this mesh corresponds to a Light component, mark it as an emitter
            auto l = lightComp;
            GLint isEmitterLoc = glGetUniformLocation(shader->id, "isEmitter");
            if (isEmitterLoc >= 0)
            {
                if (l)
                    glUniform1i(isEmitterLoc, 1);
                else
                    glUniform1i(isEmitterLoc, 0);
            }
        }

        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void RenderSystem::Cleanup()
{
    delete shader;
}
