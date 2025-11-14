#include "ecs/CameraSystem.hpp"
#include <glm/glm.hpp>

void CameraSystem::Update(Registry &registry, float dt)
{
    if (enabled && !*enabled)
        return;

    for (auto [e, cam] : registry.View<Camera>())
    {
        float dx = 0.0f, dy = 0.0f;
        SDL_GetRelativeMouseState(&dx, &dy);

        float xoffset = dx * cam->sensitivity;
        float yoffset = -dy * cam->sensitivity;

        cam->yaw += xoffset;
        cam->pitch += yoffset;
        cam->pitch = glm::clamp(cam->pitch, -89.0f, 89.0f);
    }
}
