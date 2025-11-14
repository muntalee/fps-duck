#include "ecs/RenderSystem.hpp"
#include "ecs/SkyboxSystem.hpp"
#include "ecs/Light.hpp"
#include "ecs/FirstPerson.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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

    // debugging: report counts once so we can see whether meshes exist
    static bool reported = false;
    if (!reported)
    {
        int meshCount = 0;
        int transformCount = 0;
        for (auto [e, m] : registry.View<Mesh>())
            ++meshCount;
        for (auto [e, t] : registry.View<Transform>())
            ++transformCount;
        std::cerr << "RenderSystem: meshes=" << meshCount << " transforms=" << transformCount << std::endl;
        reported = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Use();

    GLint vp[4] = {0, 0, 800, 600};
    glGetIntegerv(GL_VIEWPORT, vp);
    if (vp[3] > 0)
    {
        float aspect = static_cast<float>(vp[2]) / static_cast<float>(vp[3]);
        proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

    // update camera view matrix from any Camera component, then upload
    for (auto [e, cam] : registry.View<Camera>())
    {
        view = cam->GetViewMatrix();
        break;
    }

    shader->SetMat4("view", &view[0][0]);

    // avoid stretching
    ImGuiIO &io = ImGui::GetIO();
    float width = io.DisplaySize.x > 0.0f ? io.DisplaySize.x : 800.0f;
    float height = io.DisplaySize.y > 0.0f ? io.DisplaySize.y : 600.0f;
    proj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
    shader->SetMat4("proj", &proj[0][0]);

    // find first light in scene
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

    // render skybox (if set) after clearing and camera/projection are updated
    if (skybox)
    {
        skybox->Update(registry, dt);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            std::cerr << "RenderSystem: GL error after skybox draw: 0x" << std::hex << err << std::dec << std::endl;
    }

    // ensure our main shader is active again
    shader->Use();

    // make sure texture unit 0 is active and cube map unbound so mesh draws behave predictably
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    for (auto [e, transform] : registry.View<Transform>())
    {
        auto mesh = registry.GetComponent<Mesh>(e);
        if (!mesh)
            continue;

        glm::mat4 modelMat = transform->GetMatrix();
        // if entity is first-person, render it in camera (view) space so it stays fixed on-screen
        if (registry.GetComponent<FirstPerson>(e))
        {
            // model in world space = inverse(view) * model_in_camera_space
            modelMat = glm::inverse(view) * modelMat;
        }

        shader->SetMat4("model", &modelMat[0][0]);
        // bind mesh VAO and check GL state
        glBindVertexArray(mesh->vao);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "RenderSystem: GL error after binding VAO for entity " << e << ": 0x" << std::hex << err << std::dec << std::endl;
        }
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
                // use mesh color when no texture and no explicit light component
                if (objColorLoc >= 0)
                    glUniform3f(objColorLoc, mesh->color.r, mesh->color.g, mesh->color.b);
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

        // draw and check for errors (report entity and VAO/indexCount)
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "RenderSystem: GL error after glDrawElements for entity " << e << " (vao=" << mesh->vao << " idxCount=" << mesh->indexCount << "): 0x" << std::hex << err << std::dec << std::endl;
        }
        glBindVertexArray(0);
    }
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::cerr << "RenderSystem: GL error after mesh draws: 0x" << std::hex << err << std::dec << std::endl;
}

void RenderSystem::Cleanup()
{
    delete shader;
}
