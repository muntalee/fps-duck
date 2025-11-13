#pragma once
#include "ecs/System.hpp"
#include <imgui.h>

class LightSystem : public System
{
    bool *showUI = nullptr;

public:
    LightSystem(bool *show = nullptr) : showUI(show) {}
    void Update(Registry &registry, float dt) override;
};
