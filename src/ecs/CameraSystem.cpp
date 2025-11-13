#include "ecs/CameraSystem.hpp"
#include <glm/glm.hpp>

void CameraSystem::Update(Registry &registry, float dt)
{
    if (enabled && !*enabled)
        return;

    const bool *state = SDL_GetKeyboardState(nullptr);

    for (auto [e, cam] : registry.View<Camera>())
    {
        const float moveSpeed = cam->speed * dt;

        // orientation vectors
        glm::vec3 front;
        front.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        front.y = sin(glm::radians(cam->pitch));
        front.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        front = glm::normalize(front);

        const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        // movement constrained to XZ plane
        glm::vec3 forwardXZ(front.x, 0.0f, front.z);

        if (glm::dot(forwardXZ, forwardXZ) < 1e-6f)
            forwardXZ = glm::vec3(0.0f, 0.0f, -1.0f);
        else
            forwardXZ = glm::normalize(forwardXZ);

        glm::vec3 rightXZ = glm::normalize(glm::cross(forwardXZ, worldUp));

        // accumulate input into a single delta, normalize to avoid faster diagonal movement
        glm::vec3 delta(0.0f);
        if (state[SDL_SCANCODE_W])
            delta += forwardXZ;
        if (state[SDL_SCANCODE_S])
            delta -= forwardXZ;
        if (state[SDL_SCANCODE_D])
            delta += rightXZ;
        if (state[SDL_SCANCODE_A])
            delta -= rightXZ;
        if (state[SDL_SCANCODE_SPACE])
            delta += worldUp;
        if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
            delta -= worldUp;

        if (glm::dot(delta, delta) > 1e-8f)
            cam->pos += glm::normalize(delta) * moveSpeed;

        float dx = 0.0f, dy = 0.0f;
        SDL_GetRelativeMouseState(&dx, &dy);

        float xoffset = dx * cam->sensitivity;
        float yoffset = -dy * cam->sensitivity;

        cam->yaw += xoffset;
        cam->pitch += yoffset;
        cam->pitch = glm::clamp(cam->pitch, -89.0f, 89.0f);
    }
}
