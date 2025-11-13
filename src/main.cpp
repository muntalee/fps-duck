#include "renderer/Window.hpp"
#include "renderer/Renderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/System.hpp"

struct Position
{
    glm::vec3 pos;
};

class DemoSystem : public System
{
public:
    void Update(Registry &registry, float dt) override
    {
        ImGui::Begin("ECS Demo");
        for (auto [e, p] : registry.View<Position>())
        {
            ImGui::Text("Entity %u at (%.2f, %.2f, %.2f)", e, p->pos.x, p->pos.y, p->pos.z);
            p->pos.x += 0.25f * dt;
        }
        ImGui::End();
    }
};

int main()
{
    Window window;
    if (!window.Init("FPS Duck", 800, 600))
        return -1;

    Registry registry;
    DemoSystem demo;
    Renderer renderer;

    // epic entities
    for (int i = 0; i < 5; ++i)
    {
        Entity e = registry.CreateEntity();
        registry.AddComponent<Position>(e, {{(float)i, 0, 0}});
    }

    bool running = true;
    Uint64 prevTicks = SDL_GetTicks();

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
        demo.Update(registry, dt);
        renderer.RenderScene(registry);
        window.EndFrame();
    }

    window.Cleanup();
    return 0;
}
