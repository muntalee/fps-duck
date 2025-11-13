#include "ecs/CameraSystem.hpp"
#include <glm/glm.hpp>

void CameraSystem::Update(Registry &registry, float dt)
{
    const bool *state = SDL_GetKeyboardState(nullptr);

    for (auto [e, cam] : registry.View<Camera>())
    {
        glm::vec3 front;
        front.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        front.y = sin(glm::radians(cam->pitch));
        front.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        front = glm::normalize(front);
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));

        // WASD
        if (state[SDL_SCANCODE_W])
            cam->pos += front * cam->speed * dt;
        if (state[SDL_SCANCODE_S])
            cam->pos -= front * cam->speed * dt;
        if (state[SDL_SCANCODE_A])
            cam->pos -= right * cam->speed * dt;
        if (state[SDL_SCANCODE_D])
            cam->pos += right * cam->speed * dt;

        // mouse look
        float x, y;
        Uint32 mouseButtons = SDL_GetMouseState(&x, &y);
        if (mouseButtons & SDL_BUTTON_LMASK)
        {
            if (firstMouse)
            {
                lastX = x;
                lastY = y;
                firstMouse = false;
            }
            float xoffset = x - lastX;
            float yoffset = lastY - y; // reversed
            lastX = x;
            lastY = y;

            xoffset *= cam->sensitivity;
            yoffset *= cam->sensitivity;

            cam->yaw += xoffset;
            cam->pitch += yoffset;
            cam->pitch = glm::clamp(cam->pitch, -89.0f, 89.0f);
        }
        else
        {
            firstMouse = true;
        }
    }
}
