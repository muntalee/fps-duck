#include "ecs/CollisionSystem.hpp"
#include "ecs/Collider.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Velocity.hpp"
#include "ecs/Bullet.hpp"
#include "ecs/Player.hpp"
#include "ecs/Camera.hpp"
#include "ecs/Mesh.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

// simple AABB vs AABB overlap test in world space
static bool AABBOverlap(const glm::vec3 &posA, const glm::vec3 &halfA, const glm::vec3 &posB, const glm::vec3 &halfB)
{
    return (std::abs(posA.x - posB.x) <= (halfA.x + halfB.x)) &&
           (std::abs(posA.y - posB.y) <= (halfA.y + halfB.y)) &&
           (std::abs(posA.z - posB.z) <= (halfA.z + halfB.z));
}

// sphere vs AABB test
static bool SphereAABBOverlap(const glm::vec3 &spos, float radius, const glm::vec3 &bpos, const glm::vec3 &bhalf)
{
    // find closest point on AABB to sphere center
    glm::vec3 d;
    d.x = std::max(bpos.x - bhalf.x, std::min(spos.x, bpos.x + bhalf.x));
    d.y = std::max(bpos.y - bhalf.y, std::min(spos.y, bpos.y + bhalf.y));
    d.z = std::max(bpos.z - bhalf.z, std::min(spos.z, bpos.z + bhalf.z));
    float dist2 = (d.x - spos.x) * (d.x - spos.x) + (d.y - spos.y) * (d.y - spos.y) + (d.z - spos.z) * (d.z - spos.z);
    return dist2 <= radius * radius;
}

void CollisionSystem::Update(Registry &registry, float dt)
{
    // first, handle bullets: detect collision with any AABB colliders and remove bullet on hit
    for (auto [be, bptr] : registry.View<Bullet>())
    {
        auto bt = registry.GetComponent<Transform>(be);
        auto bcol = registry.GetComponent<Collider>(be);
        if (!bt || !bcol)
            continue;

        // test against all AABB colliders
        for (auto [ce, col] : registry.View<Collider>())
        {
            if (ce == be)
                continue;
            if (col->type != Collider::AABB)
                continue;
            auto ctrans = registry.GetComponent<Transform>(ce);
            if (!ctrans)
                continue;

            if (SphereAABBOverlap(bt->position, bcol->radius, ctrans->position, col->halfExtents))
            {
                // remove bullet visual/physics components
                registry.RemoveComponent<Mesh>(be);
                registry.RemoveComponent<Transform>(be);
                registry.RemoveComponent<Bullet>(be);
                registry.RemoveComponent<Collider>(be);
                registry.RemoveComponent<Velocity>(be);
                break;
            }
        }
    }

    // gravity and player collision
    const float gravity = 9.81f;

    for (auto [e, player] : registry.View<Player>())
    {
        if (!player)
            continue;

        // assume player entity has a Camera and a Transform (camera pos in Camera component)
        Camera *cam = registry.GetComponent<Camera>(e);
        if (!cam)
            continue;

        // ensure velocity component exists
        Velocity *vel = registry.GetComponent<Velocity>(e);
        if (!vel)
        {
            Velocity v;
            registry.AddComponent<Velocity>(e, v);
            vel = registry.GetComponent<Velocity>(e);
        }

        // apply gravity
        vel->v.y -= gravity * dt;

        // integrate vertical movement only
        cam->pos.y += vel->v.y * dt;

        // do a simple ground/cube collision test: find any AABB whose top is at or above camera bottom
        // player's half height
        const float playerHalfHeight = 0.9f;
        for (auto [ce, col] : registry.View<Collider>())
        {
            if (col->type != Collider::AABB)
                continue;
            auto t = registry.GetComponent<Transform>(ce);
            if (!t)
                continue;

            // check XZ overlap first
            glm::vec3 aPos = cam->pos;
            glm::vec3 aHalf = glm::vec3(0.3f, playerHalfHeight, 0.3f);
            glm::vec3 bPos = t->position;
            glm::vec3 bHalf = col->halfExtents;

            // enlarge bHalf.y slightly to ensure contact
            if ((std::abs(aPos.x - bPos.x) <= (aHalf.x + bHalf.x)) && (std::abs(aPos.z - bPos.z) <= (aHalf.z + bHalf.z)))
            {
                float groundTop = bPos.y + bHalf.y;
                float playerBottom = cam->pos.y - playerHalfHeight;
                if (playerBottom < groundTop)
                {
                    // push player up onto surface
                    cam->pos.y = groundTop + playerHalfHeight;
                    vel->v.y = 0.0f;
                }
            }
        }
    }
}
