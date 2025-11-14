#include "ecs/MovementSystem.hpp"
#include "ecs/Camera.hpp"
#include "ecs/Velocity.hpp"
#include "ecs/Collider.hpp"
#include "ecs/Transform.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

// simple AABB overlap used for predictive collision checks
static bool AABBOverlap(const glm::vec3 &posA, const glm::vec3 &halfA, const glm::vec3 &posB, const glm::vec3 &halfB)
{
    return (std::abs(posA.x - posB.x) <= (halfA.x + halfB.x)) &&
           (std::abs(posA.y - posB.y) <= (halfA.y + halfB.y)) &&
           (std::abs(posA.z - posB.z) <= (halfA.z + halfB.z));
}

void MovementSystem::Update(Registry &registry, float dt)
{
    auto state = SDL_GetKeyboardState(nullptr);

    for (auto [e, cam] : registry.View<Camera>())
    {
        // compute orientation-based axes
        glm::vec3 front;
        front.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        front.y = 0.0f;
        front.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        if (glm::dot(front, front) < 1e-6f)
            front = glm::vec3(0.0f, 0.0f, -1.0f);
        else
            front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));

        glm::vec3 moveDir(0.0f);
        if (state[SDL_SCANCODE_W])
            moveDir += front;
        if (state[SDL_SCANCODE_S])
            moveDir -= front;
        if (state[SDL_SCANCODE_D])
            moveDir += right;
        if (state[SDL_SCANCODE_A])
            moveDir -= right;

        glm::vec3 intendedMove(0.0f);
        if (glm::dot(moveDir, moveDir) > 1e-6f)
            intendedMove = glm::normalize(moveDir) * cam->speed * dt;

        // predictive collision: try X and Z separately to allow sliding
        Collider *pcol = registry.GetComponent<Collider>(e);
        glm::vec3 halfExt(0.3f, 0.9f, 0.3f);
        if (pcol)
            halfExt = pcol->halfExtents;

        glm::vec3 newPos = cam->pos;

        // try X movement
        if (intendedMove.x != 0.0f)
        {
            glm::vec3 testPos = newPos + glm::vec3(intendedMove.x, 0.0f, 0.0f);
            bool blocked = false;
            for (auto [ce, col] : registry.View<Collider>())
            {
                if (ce == e)
                    continue;
                if (col->type != Collider::AABB)
                    continue;
                // ignore very thin colliders (ground plane) for horizontal blocking so the player
                // can walk freely while standing on them
                if (col->halfExtents.y < 0.2f)
                    continue;
                auto t = registry.GetComponent<Transform>(ce);
                if (!t)
                    continue;
                // only consider XZ overlap for horizontal movement blocking so standing on ground doesn't block movement
                if ((std::abs(testPos.x - t->position.x) <= (halfExt.x + col->halfExtents.x)) &&
                    (std::abs(testPos.z - t->position.z) <= (halfExt.z + col->halfExtents.z)))
                {
                    blocked = true;
                    break;
                }
            }
            if (!blocked)
                newPos.x += intendedMove.x;
        }

        // try Z movement
        if (intendedMove.z != 0.0f)
        {
            glm::vec3 testPos = newPos + glm::vec3(0.0f, 0.0f, intendedMove.z);
            bool blocked = false;
            for (auto [ce, col] : registry.View<Collider>())
            {
                if (ce == e)
                    continue;
                if (col->type != Collider::AABB)
                    continue;
                // ignore very thin colliders (ground plane) for horizontal blocking
                if (col->halfExtents.y < 0.2f)
                    continue;
                auto t = registry.GetComponent<Transform>(ce);
                if (!t)
                    continue;
                // only consider XZ overlap for horizontal movement blocking
                if ((std::abs(testPos.x - t->position.x) <= (halfExt.x + col->halfExtents.x)) &&
                    (std::abs(testPos.z - t->position.z) <= (halfExt.z + col->halfExtents.z)))
                {
                    blocked = true;
                    break;
                }
            }
            if (!blocked)
                newPos.z += intendedMove.z;
        }

        cam->pos.x = newPos.x;
        cam->pos.z = newPos.z;

        // Jumping: only allow jump if player has a Velocity component and is effectively on ground
        Velocity *vel = registry.GetComponent<Velocity>(e);
        if (!vel)
        {
            Velocity v;
            registry.AddComponent<Velocity>(e, v);
            vel = registry.GetComponent<Velocity>(e);
        }

        // grounded if vertical velocity is approximately zero
        bool grounded = std::abs(vel->v.y) < 1e-3f;
        if (state[SDL_SCANCODE_SPACE] && grounded)
        {
            // set upward impulse
            vel->v.y = 5.0f; // jump impulse, tweakable
        }
    }
}
