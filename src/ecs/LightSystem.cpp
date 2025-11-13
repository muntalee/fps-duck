#include "ecs/LightSystem.hpp"
#include "ecs/Light.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Registry.hpp"
#include <imgui_internal.h>

void LightSystem::Update(Registry &registry, float dt)
{
    if (!showUI || !*showUI)
        return;

    ImGuiContext *ctx = ImGui::GetCurrentContext();
    if (!ctx || !ctx->WithinFrameScope)
        return;

    // ui for lighting
    for (auto [e, light] : registry.View<Light>())
    {
        auto transform = registry.GetComponent<Transform>(e);
        if (!transform)
            continue;
        ImGui::Begin("Light");
        ImGui::ColorEdit3("Color", &light->color.x);
        ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
        ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 10.0f);
        ImGui::End();
        break;
    }
}
