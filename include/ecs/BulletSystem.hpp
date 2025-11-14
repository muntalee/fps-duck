#pragma once
#include "ecs/System.hpp"
#include "ecs/Mesh.hpp"
#include <glm/glm.hpp>

class BulletSystem : public System
{
    Mesh sphereMesh;

public:
    BulletSystem();
    void Update(Registry &registry, float dt) override;
    void SpawnBullet(Registry &registry, const glm::vec3 &pos, const glm::vec3 &dir, float speed = 30.0f, float ttl = 5.0f);
};
