#include "renderer/Window.hpp"
#include "ecs/Registry.hpp"
#include "ecs/System.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Mesh.hpp"
#include "ecs/RenderSystem.hpp"
#include "ecs/Camera.hpp"
#include "ecs/CameraSystem.hpp"
#include <glm/glm.hpp>
#include <imgui.h>

struct Position
{
    glm::vec3 pos;
};

class DemoSystem : public System
{
public:
    void Update(Registry &registry, float dt) override
    {
        ImGui::Begin("joemama");
        ImGui::End();
    }
};

// sample cube
Mesh CreateCube()
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f};
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0, 2, 3, 7, 7, 6, 2,
        1, 2, 6, 6, 5, 1, 3, 0, 4, 4, 7, 3};

    Mesh mesh;
    // create and bind VAO/VBO/EBO before uploading data
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // unbind VAO (EBO remains bound to VAO, so unbinding VAO will keep state saved)
    glBindVertexArray(0);

    mesh.indexCount = static_cast<int>(sizeof(indices) / sizeof(unsigned int));
    return mesh;
}

int main()
{
    Window window;
    if (!window.Init("FPS Duck", 800, 600))
        return -1;

    Registry registry;
    DemoSystem demo;
    RenderSystem renderSystem;

    // rotating cube
    Entity cube = registry.CreateEntity();
    registry.AddComponent<Transform>(cube, {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}});
    registry.AddComponent<Mesh>(cube, CreateCube());

    // Create demo ECS-only entities
    for (int i = 0; i < 5; ++i)
    {
        Entity e = registry.CreateEntity();
        registry.AddComponent<Position>(e, {{(float)i, 0, 0}});
    }

    bool running = true;
    Uint64 prevTicks = SDL_GetTicks();

    CameraSystem cameraSystem;

    // create camera entity
    Entity camEntity = registry.CreateEntity();
    registry.AddComponent<Camera>(camEntity, {});

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        Uint64 now = SDL_GetTicks();
        float dt = (now - prevTicks) / 1000.0f;
        prevTicks = now;

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
