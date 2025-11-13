#pragma once
#include "ecs/System.hpp"
#include <glm/glm.hpp>

class FirstPersonSystem : public System
{
    // offsets and default transform parameters for first-person models
    float forwardDistance = 0.6f;
    float rightOffset = 0.25f;
    float downOffset = -0.25f;
    glm::vec3 baseRotation = {0.0f, -90.0f, 0.0f};
    glm::vec3 baseScale = {1.2f, 1.2f, 1.2f};

    // pitch control
    float pitchInfluence = 0.25f; // 0 = no pitch, 1 = full camera pitch
    float maxPitchDelta = 3.0f;   // clamp delta from baseRotation.x (degrees)
    float smoothSpeed = 10.0f;    // how fast the gun aims toward target pitch

public:
    FirstPersonSystem() = default;
    void Update(Registry &registry, float dt) override;

    // setters to tweak offsets at runtime if needed
    void SetOffsets(float forward, float right, float down)
    {
        forwardDistance = forward;
        rightOffset = right;
        downOffset = down;
    }
    void SetBaseRotation(const glm::vec3 &rot) { baseRotation = rot; }
    void SetBaseScale(const glm::vec3 &s) { baseScale = s; }
    void SetPitchInfluence(float v) { pitchInfluence = v; }
    void SetMaxPitchDelta(float v) { maxPitchDelta = v; }
    void SetSmoothSpeed(float v) { smoothSpeed = v; }
};
