#include "ecs/PlayerSystem.hpp"
#include "ecs/Player.hpp"
#include "ecs/Camera.hpp"
#include "ecs/Transform.hpp"
#include "ecs/FirstPerson.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

void PlayerSystem::Update(Registry &registry, float dt)
{
    for (auto [e, p] : registry.View<Player>())
    {
        if (!p)
            continue;

        if (p->cooldown > 0.0f)
            p->cooldown = std::max(0.0f, p->cooldown - dt);

        Camera *cam = registry.GetComponent<Camera>(e);
        if (!cam)
            continue;

        if (p->wantFire && p->cooldown <= 0.0f && bulletSystem)
        {
            // compute forward direction from camera yaw/pitch
            glm::vec3 front;
            front.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
            front.y = sin(glm::radians(cam->pitch));
            front.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
            glm::vec3 dir = glm::normalize(front);

            glm::vec3 spawnPos = cam->pos + dir * 0.6f;
            bulletSystem->SpawnBullet(registry, spawnPos, dir, 40.0f, 4.0f);
            p->cooldown = 1.0f / p->fireRate;
        }

        // clear fire request each frame
        p->wantFire = false;
    }
}
