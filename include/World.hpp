#pragma once
#include <string>
#include "ecs/Registry.hpp"

namespace World
{
    void LoadFromFile(Registry &registry, const std::string &path, float tileSize = 1.0f);
}
