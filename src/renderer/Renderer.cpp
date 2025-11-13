#include "renderer/Renderer.hpp"
#include "ecs/Registry.hpp"

void Renderer::RenderScene(Registry &registry)
{
  ImGui::Begin("Scene Renderer");
  ImGui::Text("Rendering %zu entities...", registry.View<glm::vec3>().size());
  ImGui::End();
}
