#include "ecs/FirstPersonSystem.hpp"
#include "ecs/Camera.hpp"
#include "ecs/Transform.hpp"
#include "ecs/FirstPerson.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void FirstPersonSystem::Update(Registry &registry, float dt)
{
    // find the camera component
    Camera *cam = nullptr;
    for (auto [e, c] : registry.View<Camera>())
    {
        cam = c;
        break;
    }
    if (!cam)
        return;

    // update every entity that has FirstPerson + Transform
    for (auto [e, fp] : registry.View<FirstPerson>())
    {
        auto t = registry.GetComponent<Transform>(e);
        if (!t)
            continue;

        // store transform in camera-local coordinates: (right, up, -forward)
        t->position = glm::vec3(rightOffset, downOffset, -forwardDistance);

        // apply pitch influence, clamp and smooth toward target to avoid extreme rotations
        float targetPitch = baseRotation.x + cam->pitch * pitchInfluence;
        targetPitch = glm::clamp(targetPitch, baseRotation.x - maxPitchDelta, baseRotation.x + maxPitchDelta);

        float lerpT = glm::clamp(smoothSpeed * dt, 0.0f, 1.0f);
        t->rotation.x = glm::mix(t->rotation.x, targetPitch, lerpT);

        // keep yaw and roll locked (model-space alignment)
        t->rotation.y = baseRotation.y;
        t->rotation.z = baseRotation.z;

        t->scale = baseScale;
    }
}
