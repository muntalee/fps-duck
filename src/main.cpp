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
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include "ecs/LightSystem.hpp"
#include "ecs/Light.hpp"
#include "ecs/Model.hpp"
#include "ecs/FirstPerson.hpp"
#include "ecs/FirstPersonSystem.hpp"
#include "World.hpp"
#include "ecs/BulletSystem.hpp"
#include "ecs/Player.hpp"
#include "ecs/PlayerSystem.hpp"

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

Mesh CreateColoredCube(float size = 0.2f)
{
    float s = size * 0.5f;
    // same layout: pos, normal, uv (uv unused)
    float vertices[] = {
        // front
        -s, -s, s, 0, 0, 1, 0, 0,
        s, -s, s, 0, 0, 1, 1, 0,
        s, s, s, 0, 0, 1, 1, 1,
        -s, s, s, 0, 0, 1, 0, 1,
        // back
        -s, -s, -s, 0, 0, -1, 1, 0,
        s, -s, -s, 0, 0, -1, 0, 0,
        s, s, -s, 0, 0, -1, 0, 1,
        -s, s, -s, 0, 0, -1, 1, 1,
        // left
        -s, -s, -s, -1, 0, 0, 0, 0,
        -s, -s, s, -1, 0, 0, 1, 0,
        -s, s, s, -1, 0, 0, 1, 1,
        -s, s, -s, -1, 0, 0, 0, 1,
        // right
        s, -s, -s, 1, 0, 0, 1, 0,
        s, -s, s, 1, 0, 0, 0, 0,
        s, s, s, 1, 0, 0, 0, 1,
        s, s, -s, 1, 0, 0, 1, 1,
        // top
        -s, s, s, 0, 1, 0, 0, 0,
        s, s, s, 0, 1, 0, 1, 0,
        s, s, -s, 0, 1, 0, 1, 1,
        -s, s, -s, 0, 1, 0, 0, 1,
        // bottom
        -s, -s, s, 0, -1, 0, 0, 1,
        s, -s, s, 0, -1, 0, 1, 1,
        s, -s, -s, 0, -1, 0, 1, 0,
        -s, -s, -s, 0, -1, 0, 0, 0};

    unsigned int indices[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10,
                              11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19,
                              16, 20, 21, 22, 22, 23, 20};

    Mesh mesh;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    mesh.indexCount = static_cast<int>(sizeof(indices) / sizeof(unsigned int));
    mesh.texture = 0;
    return mesh;
}

int main()
{
    Window window;
    if (!window.Init("FPS Duck", 1280, 720))
        return -1;

    Registry registry;
    bool showUI = false;
    bool inputCaptured = true;

    DemoSystem demo(&showUI);
    RenderSystem renderSystem;
    BulletSystem bulletSystem;
    PlayerSystem playerSystem(&bulletSystem);
    FirstPersonSystem fpSystem;

    // load gun model from data/gun (OBJ + MTL + textures expected)
    Entity gun = registry.CreateEntity();
    Mesh gunMesh = Model::LoadFromOBJ("data/gun/gun.obj");
    registry.AddComponent<Mesh>(gun, gunMesh);

    // me when camera
    registry.AddComponent<FirstPerson>(gun, {});
    registry.AddComponent<Transform>(gun, {});

    bool running = true;
    Uint64 prevTicks = SDL_GetTicks();

    CameraSystem cameraSystem;
    cameraSystem.SetEnabled(&inputCaptured);

    // start with mouse captured for FPS look
    SDL_SetWindowRelativeMouseMode(window.window, inputCaptured);

    // light system
    LightSystem lightSystem(&showUI);
    Entity lightEntity = registry.CreateEntity();
    registry.AddComponent<Transform>(lightEntity, {{8.0f, 8.0f, 8.0f}, {0, 0, 0}, {0.6f, 0.6f, 0.6f}});
    registry.AddComponent<Light>(lightEntity, {{1.0f, 0.95f, 0.8f}, 2.0f});
    registry.AddComponent<Mesh>(lightEntity, CreateColoredCube(0.6f));

    // create camera entity
    Entity camEntity = registry.CreateEntity();
    registry.AddComponent<Camera>(camEntity, {});

    // player entity
    Player playerComp;
    playerComp.gun = gun;
    registry.AddComponent<Player>(camEntity, playerComp);

    // the world
    World::LoadFromFile(registry, "data/world.txt", 1.0f);

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

            // fire on left mouse button
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && inputCaptured)
            {
                auto p = registry.GetComponent<Player>(camEntity);
                if (p)
                    p->wantFire = true;
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

        // update systems
        demo.Update(registry, dt);
        lightSystem.Update(registry, dt);
        cameraSystem.Update(registry, dt);
        fpSystem.Update(registry, dt);
        playerSystem.Update(registry, dt);
        bulletSystem.Update(registry, dt);
        renderSystem.Update(registry, dt);

        window.EndFrame();
    }

    renderSystem.Cleanup();
    window.Cleanup();
    return 0;
}
