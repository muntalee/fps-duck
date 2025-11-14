#include "ecs/BulletSystem.hpp"
#include "ecs/Bullet.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Mesh.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Collider.hpp"
#include <vector>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <iostream>

// simple UV sphere mesh for bullet visualization
static Mesh CreateSphereMesh(int lat = 6, int lon = 6)
{
    std::vector<float> verts;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= lat; ++y)
    {
        float v = (float)y / (float)lat;
        float theta = v * glm::pi<float>();
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int x = 0; x <= lon; ++x)
        {
            float u = (float)x / (float)lon;
            float phi = u * glm::two_pi<float>();
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float px = sinTheta * cosPhi;
            float py = cosTheta;
            float pz = sinTheta * sinPhi;

            // position
            verts.push_back(px);
            verts.push_back(py);
            verts.push_back(pz);
            // normal
            verts.push_back(px);
            verts.push_back(py);
            verts.push_back(pz);
            // uv
            verts.push_back(u);
            verts.push_back(v);
        }
    }

    for (int y = 0; y < lat; ++y)
    {
        for (int x = 0; x < lon; ++x)
        {
            int a = (y * (lon + 1)) + x;
            int b = a + lon + 1;
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(b);
            indices.push_back(b + 1);
            indices.push_back(a + 1);
        }
    }

    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    m.indexCount = static_cast<int>(indices.size());
    m.texture = 0;
    m.color = glm::vec3(1.0f, 0.15f, 0.15f); // red
    return m;
}

BulletSystem::BulletSystem()
{
    sphereMesh = CreateSphereMesh(8, 12);
}

void BulletSystem::Update(Registry &registry, float dt)
{
    // iterate all bullets and update position
    for (auto [e, b] : registry.View<Bullet>())
    {
        auto t = registry.GetComponent<Transform>(e);
        if (!t)
            continue;
        // move
        t->position += b->dir * b->speed * dt;
        b->ttl -= dt;
        // lifetime expired
        if (b->ttl <= 0.0f)
        {
            registry.RemoveComponent<Mesh>(e);
            registry.RemoveComponent<Transform>(e);
            registry.RemoveComponent<Bullet>(e);
            continue;
        }
    }
}

void BulletSystem::SpawnBullet(Registry &registry, const glm::vec3 &pos, const glm::vec3 &dir, float speed, float ttl)
{
    Entity e = registry.CreateEntity();
    Transform t;
    t.position = pos;
    t.scale = glm::vec3(0.07f);
    registry.AddComponent<Transform>(e, t);

    Mesh m = sphereMesh;
    registry.AddComponent<Mesh>(e, m);

    Bullet b;
    b.dir = glm::normalize(dir);
    b.speed = speed;
    b.ttl = ttl;
    registry.AddComponent<Bullet>(e, b);

    Collider col;
    col.type = Collider::Sphere;
    col.radius = 0.07f;
    registry.AddComponent<Collider>(e, col);
}
