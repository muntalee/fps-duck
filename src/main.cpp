#include "renderer/Window.hpp"
#include "ecs/Registry.hpp"
#include "ecs/System.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Mesh.hpp"
#include "ecs/RenderSystem.hpp"
#include "ecs/Camera.hpp"
#include "ecs/CameraSystem.hpp"
#include "ecs/Texture.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>

struct Position
{
    glm::vec3 pos;
};

class DemoSystem : public System
{
    bool *showUI = nullptr;

public:
    DemoSystem(bool *show = nullptr) : showUI(show) {}

    void Update(Registry &registry, float dt) override
    {
        if (!showUI || !*showUI)
            return;

        ImGuiContext *ctx = ImGui::GetCurrentContext();
        if (!ctx || !ctx->WithinFrameScope)
            return;

        ImGui::Begin("ECS Demo");

        ImGuiIO &io = ImGui::GetIO();
        float fps = io.Framerate;
        ImGui::Text("FPS: %.1f (%.2f ms/frame)", fps, fps > 0.0f ? 1000.0f / fps : 0.0f);
        ImGui::Text("Delta time: %.4f s", dt);

        int posCount = 0;
        for (auto [e, p] : registry.View<Position>())
            ++posCount;
        int transformCount = 0;
        for (auto [e, t] : registry.View<Transform>())
            ++transformCount;
        int meshCount = 0;
        for (auto [e, m] : registry.View<Mesh>())
            ++meshCount;
        int cameraCount = 0;
        for (auto [e, c] : registry.View<Camera>())
            ++cameraCount;

        ImGui::Separator();
        ImGui::Text("Component counts:");
        ImGui::BulletText("Position: %d", posCount);
        ImGui::BulletText("Transform: %d", transformCount);
        ImGui::BulletText("Mesh: %d", meshCount);
        ImGui::BulletText("Camera: %d", cameraCount);

        ImGui::Separator();
        ImGui::Text("Runtime state:");
        ImGui::Text("Menu visible: %s", (*showUI) ? "Yes" : "No");
        ImGui::Text("Mouse captured: %s", (*showUI) ? "No" : "Yes");

        // debug: list first 5 transforms
        int shown = 0;
        ImGui::Separator();
        ImGui::Text("Transforms (first 5):");
        for (auto [e, t] : registry.View<Transform>())
        {
            if (shown++ >= 5)
                break;
            ImGui::Text("E%u pos=(%.2f, %.2f, %.2f) rot=(%.1f, %.1f, %.1f) scale=(%.2f, %.2f, %.2f)",
                        e,
                        t->position.x, t->position.y, t->position.z,
                        t->rotation.x, t->rotation.y, t->rotation.z,
                        t->scale.x, t->scale.y, t->scale.z);
        }

        ImGui::End();
    }
};

// sample cube with UVs (24 vertices, 36 indices)
Mesh CreateCube()
{
    // positions (x,y,z) and texcoords (u,v) for each face (4 verts per face)
    float vertices[] = {
        // front
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        // back
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        // left
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        // right
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        // top
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        // bottom
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f};

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20};

    Mesh mesh;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    mesh.indexCount = static_cast<int>(sizeof(indices) / sizeof(unsigned int));

    // load texture from data/ (copy to build dir required)
    mesh.texture = Texture::Load("data/test.jpg");
    return mesh;
}

int main()
{
    Window window;
    if (!window.Init("FPS Duck", 800, 600))
        return -1;

    Registry registry;
    bool showUI = false;       // false = playing (camera captured), true = menu shown
    bool inputCaptured = true; // when true, camera captures mouse & processes input

    DemoSystem demo(&showUI);
    RenderSystem renderSystem;

    // rotating cube
    Entity cube = registry.CreateEntity();
    registry.AddComponent<Transform>(cube, {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}});
    registry.AddComponent<Mesh>(cube, CreateCube());

    bool running = true;
    Uint64 prevTicks = SDL_GetTicks();

    CameraSystem cameraSystem;
    cameraSystem.SetEnabled(&inputCaptured);
    // start with mouse captured for FPS look
    SDL_SetWindowRelativeMouseMode(window.window, inputCaptured);

    // create camera entity
    Entity camEntity = registry.CreateEntity();
    registry.AddComponent<Camera>(camEntity, {});

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            // toggle menu / capture on escapes
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
            {
                showUI = !showUI;
                inputCaptured = !showUI;
                // enable/disable relative mouse mode on the window
                SDL_SetWindowRelativeMouseMode(window.window, inputCaptured);
            }
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        Uint64 now = SDL_GetTicks();
        float dt = (now - prevTicks) / 1000.0f;
        prevTicks = now;

        // if menu is visible, show an escape-menu with framerate
        if (showUI)
        {
            ImGuiContext *ctx = ImGui::GetCurrentContext();
            if (ctx && ctx->WithinFrameScope)
            {
                ImGui::Begin("Escape Menu");
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                if (ImGui::Button("Resume (ESC)"))
                {
                    showUI = false;
                    inputCaptured = true;
                    SDL_SetWindowRelativeMouseMode(window.window, inputCaptured);
                }
                ImGui::End();
            }
        }

        window.BeginFrame();

        // rotate cube
        if (auto *t = registry.GetComponent<Transform>(cube))
        {
            t->rotation.y += 50.0f * dt;
            if (t->rotation.y > 360.0f)
                t->rotation.y -= 360.0f;
        }

        demo.Update(registry, dt);
        cameraSystem.Update(registry, dt);
        renderSystem.Update(registry, dt);

        window.EndFrame();
    }

    renderSystem.Cleanup();
    window.Cleanup();
    return 0;
}
