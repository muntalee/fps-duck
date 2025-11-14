#include "World.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "ecs/Mesh.hpp"
#include "ecs/Transform.hpp"

namespace
{
    // create a simple plane mesh centered at origin on the XZ plane (y = 0)
    Mesh CreatePlaneMesh(float width, float depth)
    {
        float hw = width * 0.5f;
        float hd = depth * 0.5f;
        float vertices[] = {
            -hw,
            0.0f,
            -hd,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            hw,
            0.0f,
            -hd,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f,
            hw,
            0.0f,
            hd,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            -hw,
            0.0f,
            hd,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
        };
        unsigned int indices[] = {0, 1, 2, 2, 3, 0};

        Mesh mesh;
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ebo);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        mesh.indexCount = static_cast<int>(sizeof(indices) / sizeof(unsigned int));
        mesh.texture = 0;
        mesh.color = glm::vec3(0.15f, 0.8f, 0.25f); // grassy green
        return mesh;
    }

    // create a unit cube mesh (centered at origin, size 1)
    Mesh CreateUnitCube()
    {
        float vertices[] = {
            // front (+Z)
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            // back (-Z)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            // left (-X)
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            // right (+X)
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            // top (+Y)
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            // bottom (-Y)
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f};

        unsigned int indices[] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20};

        Mesh mesh;
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ebo);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        mesh.indexCount = static_cast<int>(sizeof(indices) / sizeof(unsigned int));
        mesh.texture = 0;
        mesh.color = glm::vec3(0.7f, 0.7f, 0.7f);
        return mesh;
    }
}

namespace World
{
    void LoadFromFile(Registry &registry, const std::string &path, float tileSize)
    {
        std::ifstream in(path);
        if (!in)
        {
            std::cerr << "World: failed to open " << path << std::endl;
            return;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty())
                lines.push_back(line);
        }

        if (lines.empty())
            return;

        size_t rows = lines.size();
        size_t cols = 0;
        for (auto &l : lines)
            cols = std::max(cols, l.size());

        float width = static_cast<float>(cols) * tileSize;
        float depth = static_cast<float>(rows) * tileSize;

        // ground plane
        Mesh groundMesh = CreatePlaneMesh(width, depth);
        Entity ground = registry.CreateEntity();
        registry.AddComponent<Transform>(ground, {{0.0f, 0.0f, 0.0f}, {0, 0, 0}, {1, 1, 1}});
        registry.AddComponent<Mesh>(ground, groundMesh);

        // create one cube mesh and reuse it for all cubes
        Mesh cubeMesh = CreateUnitCube();
        cubeMesh.color = glm::vec3(0.6f, 0.4f, 0.2f); // earthy brown

        // center offset so map is centered around origin
        float offsetX = (static_cast<float>(cols - 1) * tileSize) * 0.5f;
        float offsetZ = (static_cast<float>(rows - 1) * tileSize) * 0.5f;

        for (size_t r = 0; r < rows; ++r)
        {
            for (size_t c = 0; c < cols; ++c)
            {
                char ch = '0';
                if (c < lines[r].size())
                    ch = lines[r][c];
                if (ch == '1')
                {
                    float x = static_cast<float>(c) * tileSize - offsetX;
                    float z = static_cast<float>(r) * tileSize - offsetZ;

                    Entity e = registry.CreateEntity();
                    // cube sits on top of ground: cube center y = tileSize/2
                    registry.AddComponent<Transform>(e, {{x, tileSize * 0.5f, z}, {0, 0, 0}, {tileSize, tileSize, tileSize}});
                    registry.AddComponent<Mesh>(e, cubeMesh);
                }
            }
        }
    }
}
