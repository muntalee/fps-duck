#pragma once
#include "Core.hpp"
#include "ecs/Registry.hpp"

class Renderer
{
public:
  void RenderScene(Registry &registry);
};
