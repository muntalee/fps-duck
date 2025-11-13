#pragma once
#include "ecs/System.hpp"
#include "ecs/Camera.hpp"
#include <SDL3/SDL.h>

class CameraSystem : public System
{
    bool firstMouse = true;
    float lastX = 400, lastY = 300;
    bool *enabled = nullptr;

public:
    void Update(Registry &registry, float dt) override;
    void SetEnabled(bool *flag) { enabled = flag; }
};
