#pragma once
#include "ecs/System.hpp"
#include "ecs/BulletSystem.hpp"

class PlayerSystem : public System
{
    BulletSystem *bulletSystem = nullptr;

public:
    PlayerSystem() = default;
    explicit PlayerSystem(BulletSystem *b) : bulletSystem(b) {}
    void Update(Registry &registry, float dt) override;
};
