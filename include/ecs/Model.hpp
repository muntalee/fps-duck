#pragma once
#include <string>
#include "ecs/Mesh.hpp"

struct Model
{
    static Mesh LoadFromOBJ(const std::string &path);
};
