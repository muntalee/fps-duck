#pragma once
#include "ecs/System.hpp"

class CollisionSystem : public System
{
public:
    CollisionSystem() = default;
    void Update(Registry &registry, float dt) override;
};
