#pragma once
#include "ecs/System.hpp"
#include <SDL3/SDL.h>

class MovementSystem : public System
{
public:
    MovementSystem() = default;
    void Update(Registry &registry, float dt) override;
};
