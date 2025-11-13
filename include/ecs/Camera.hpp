#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
    glm::vec3 pos{0.0f, 0.0f, 3.0f};
    float yaw = -90.0f; // facing -Z
    float pitch = 0.0f;
    float speed = 3.0f;
    float sensitivity = 0.1f;

    glm::mat4 GetViewMatrix() const
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::lookAt(pos, pos + glm::normalize(front), glm::vec3(0, 1, 0));
    }
};
