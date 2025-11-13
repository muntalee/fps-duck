#include "renderer/Renderer.hpp"
#include "ecs/Registry.hpp"
#include <imgui_internal.h>

void Renderer::RenderScene(Registry &registry)
{
  ImGuiContext *ctx = ImGui::GetCurrentContext();
  if (!ctx || !ctx->WithinFrameScope)
  {
    // skip ImGui calls when not inside a NewFrame/EndFrame scope.
    return;
  }

  ImGui::Begin("Scene Renderer");
  ImGui::Text("Rendering %zu entities...", registry.View<glm::vec3>().size());
  ImGui::End();
}
