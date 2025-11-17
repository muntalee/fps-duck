#pragma once

#include "ecs/System.hpp"
#include "ecs/Registry.hpp"
#include <string>
#include <vector>
#include <glm/glm.hpp>

class WorldRepeater : public System
{
public:
    WorldRepeater();
    ~WorldRepeater();

    void Configure(const std::string &mapPath, int repeats, float speed, float tileSize = 1.0f);

    void Update(Registry &registry, float dt) override;
    void Cleanup();

private:
    std::string path;
    int segmentRepeats = 0;
    float speed = 0.0f;
    float tileSize = 1.0f;
    bool initialized = false;

    std::vector<std::vector<Entity>> copies;
    float mapWidth = 0.0f;
    float mapDepth = 0.0f;
    std::vector<std::vector<glm::vec3>> copiesInitialPositions;
    double totalScroll = 0.0;
};
