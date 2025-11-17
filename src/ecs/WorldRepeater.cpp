#include "ecs/WorldRepeater.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Transform.hpp"
#include "ecs/Mesh.hpp"
#include "ecs/Texture.hpp"
#include "ecs/Collider.hpp"
#include "ecs/Camera.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace
{
    // create plane mesh matching tile grid width/depth
    Mesh CreatePlaneMesh(float width, float depth, float repeatX = 1.0f, float repeatZ = 1.0f)
    {
        float hw = width * 0.5f;
        float hd = depth * 0.5f;
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = repeatX;
        float v1 = repeatZ;
        float vertices[] = {
            -hw,
            0.0f,
            -hd,
            0.0f,
            1.0f,
            0.0f,
            u0,
            v0,
            hw,
            0.0f,
            -hd,
            0.0f,
            1.0f,
            0.0f,
            u1,
            v0,
            hw,
            0.0f,
            hd,
            0.0f,
            1.0f,
            0.0f,
            u1,
            v1,
            -hw,
            0.0f,
            hd,
            0.0f,
            1.0f,
            0.0f,
            u0,
            v1,
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
        mesh.color = glm::vec3(0.15f, 0.8f, 0.25f);
        return mesh;
    }

    Mesh CreateCubeMesh()
    {
        float vertices[] = {
            // front
            -0.5f,
            -0.5f,
            0.5f,
            0,
            0,
            1,
            0,
            0,
            0.5f,
            -0.5f,
            0.5f,
            0,
            0,
            1,
            1,
            0,
            0.5f,
            0.5f,
            0.5f,
            0,
            0,
            1,
            1,
            1,
            -0.5f,
            0.5f,
            0.5f,
            0,
            0,
            1,
            0,
            1,
            // back
            -0.5f,
            -0.5f,
            -0.5f,
            0,
            0,
            -1,
            1,
            0,
            0.5f,
            -0.5f,
            -0.5f,
            0,
            0,
            -1,
            0,
            0,
            0.5f,
            0.5f,
            -0.5f,
            0,
            0,
            -1,
            0,
            1,
            -0.5f,
            0.5f,
            -0.5f,
            0,
            0,
            -1,
            1,
            1,
            // left
            -0.5f,
            -0.5f,
            -0.5f,
            -1,
            0,
            0,
            0,
            0,
            -0.5f,
            -0.5f,
            0.5f,
            -1,
            0,
            0,
            1,
            0,
            -0.5f,
            0.5f,
            0.5f,
            -1,
            0,
            0,
            1,
            1,
            -0.5f,
            0.5f,
            -0.5f,
            -1,
            0,
            0,
            0,
            1,
            // right
            0.5f,
            -0.5f,
            -0.5f,
            1,
            0,
            0,
            1,
            0,
            0.5f,
            -0.5f,
            0.5f,
            1,
            0,
            0,
            0,
            0,
            0.5f,
            0.5f,
            0.5f,
            1,
            0,
            0,
            0,
            1,
            0.5f,
            0.5f,
            -0.5f,
            1,
            0,
            0,
            1,
            1,
            // top
            -0.5f,
            0.5f,
            0.5f,
            0,
            1,
            0,
            0,
            0,
            0.5f,
            0.5f,
            0.5f,
            0,
            1,
            0,
            1,
            0,
            0.5f,
            0.5f,
            -0.5f,
            0,
            1,
            0,
            1,
            1,
            -0.5f,
            0.5f,
            -0.5f,
            0,
            1,
            0,
            0,
            1,
            // bottom
            -0.5f,
            -0.5f,
            0.5f,
            0,
            -1,
            0,
            0,
            1,
            0.5f,
            -0.5f,
            0.5f,
            0,
            -1,
            0,
            1,
            1,
            0.5f,
            -0.5f,
            -0.5f,
            0,
            -1,
            0,
            1,
            0,
            -0.5f,
            -0.5f,
            -0.5f,
            0,
            -1,
            0,
            0,
            0,
        };
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
        mesh.color = glm::vec3(0.6f, 0.4f, 0.2f);
        return mesh;
    }

    Mesh CreateWallMesh(float width, float height, float depth)
    {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        float hd = depth * 0.5f;
        float vertices[] = {
            -hw,
            -hh,
            -hd,
            0,
            0,
            1,
            0,
            0,
            hw,
            -hh,
            -hd,
            0,
            0,
            1,
            1,
            0,
            hw,
            hh,
            -hd,
            0,
            0,
            1,
            1,
            1,
            -hw,
            hh,
            -hd,
            0,
            0,
            1,
            0,
            1,
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
        mesh.color = glm::vec3(0.3f, 0.3f, 0.8f);
        return mesh;
    }

    // create a sin-wave mesh spanning tileSize in X and depth in Z
    Mesh CreateWaveMesh(float tileSize, int segments = 24)
    {
        int seg = std::max(4, segments);
        float hw = tileSize * 0.5f;         // full tile width
        float hd = tileSize * 0.15f;        // thickness in Z (half depth)
        float amp = tileSize * 0.225f;      // amplitude (half of previous)
        float extraLift = tileSize * 0.35f; // raise crest above previous placement
        float lift = amp + extraLift;       // small lift so wave doesn't touch base exactly
        float baseY = 0.0f;                 // floor relative to mesh local origin
        int waves = 1;                      // number of wave cycles across the tile

        std::vector<float> verts;
        std::vector<unsigned int> inds;

        auto pushV = [&](float x, float y, float z, float nx, float ny, float nz, float u, float v)
        {
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
            verts.push_back(nx);
            verts.push_back(ny);
            verts.push_back(nz);
            verts.push_back(u);
            verts.push_back(v);
        };

        // top strip (triangle strip style): back/top then front/top for each sample
        const float x0 = -hw;
        const float x1 = hw;
        for (int i = 0; i <= seg; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(seg);
            float x = x0 + t * (x1 - x0);
            float y = lift + amp * sinf(2.0f * 3.14159265f * waves * t);
            // back-top (z = -hd)
            pushV(x, y, -hd, 0.0f, 1.0f, 0.0f, t, 0.0f);
            // front-top (z = +hd)
            pushV(x, y, hd, 0.0f, 1.0f, 0.0f, t, 1.0f);
        }
        unsigned int topCount = static_cast<unsigned int>((seg + 1) * 2);

        // front vertical strip (connect top to baseY at +hd)
        unsigned int frontStart = static_cast<unsigned int>(verts.size() / 8);
        for (int i = 0; i <= seg; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(seg);
            float x = x0 + t * (x1 - x0);
            float y = lift + amp * sinf(2.0f * 3.14159265f * waves * t);
            // top (front)
            pushV(x, y, hd, 0.0f, 0.0f, 1.0f, t, 0.0f);
            // bottom (front)
            pushV(x, baseY, hd, 0.0f, 0.0f, 1.0f, t, 1.0f);
        }
        unsigned int frontCount = static_cast<unsigned int>((seg + 1) * 2);

        // back vertical strip (connect top to baseY at -hd)
        unsigned int backStart = static_cast<unsigned int>(verts.size() / 8);
        for (int i = 0; i <= seg; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(seg);
            float x = x0 + t * (x1 - x0);
            float y = lift + amp * sinf(2.0f * 3.14159265f * waves * t);
            // top (back)
            pushV(x, y, -hd, 0.0f, 0.0f, -1.0f, t, 0.0f);
            // bottom (back)
            pushV(x, baseY, -hd, 0.0f, 0.0f, -1.0f, t, 1.0f);
        }
        unsigned int backCount = static_cast<unsigned int>((seg + 1) * 2);

        // bottom rectangle
        unsigned int bottomStart = static_cast<unsigned int>(verts.size() / 8);
        pushV(x0, baseY, hd, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
        pushV(x1, baseY, hd, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
        pushV(x1, baseY, -hd, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
        pushV(x0, baseY, -hd, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

        // top surface (triangle strip style): two verts per sample => quads between pairs
        for (unsigned int i = 0; i < static_cast<unsigned int>(seg); ++i)
        {
            unsigned int a = i * 2;
            unsigned int b = a + 1;
            unsigned int c = a + 2;
            unsigned int d = a + 3;
            inds.push_back(a);
            inds.push_back(b);
            inds.push_back(c);
            inds.push_back(c);
            inds.push_back(b);
            inds.push_back(d);
        }

        // front vertical strip
        unsigned int fBase = frontStart;
        for (unsigned int i = 0; i < static_cast<unsigned int>(seg); ++i)
        {
            unsigned int a = fBase + i * 2;
            unsigned int b = a + 1;
            unsigned int c = a + 2;
            unsigned int d = a + 3;
            inds.push_back(a);
            inds.push_back(c);
            inds.push_back(b);
            inds.push_back(c);
            inds.push_back(d);
            inds.push_back(b);
        }

        // back vertical strip
        unsigned int bkBase = backStart;
        for (unsigned int i = 0; i < static_cast<unsigned int>(seg); ++i)
        {
            unsigned int a = bkBase + i * 2;
            unsigned int b = a + 1;
            unsigned int c = a + 2;
            unsigned int d = a + 3;
            inds.push_back(a);
            inds.push_back(b);
            inds.push_back(c);
            inds.push_back(c);
            inds.push_back(b);
            inds.push_back(d);
        }

        // bottom face (single quad)
        inds.push_back(bottomStart + 0);
        inds.push_back(bottomStart + 1);
        inds.push_back(bottomStart + 2);
        inds.push_back(bottomStart + 2);
        inds.push_back(bottomStart + 3);
        inds.push_back(bottomStart + 0);

        // left cap (x0)
        unsigned int leftTopBack = 0;
        unsigned int leftTopFront = 1;
        unsigned int leftFrontBottom = frontStart + 1;
        unsigned int leftBackBottom = backStart + 1;
        inds.push_back(leftTopBack);
        inds.push_back(leftBackBottom);
        inds.push_back(leftTopFront);
        inds.push_back(leftTopFront);
        inds.push_back(leftBackBottom);
        inds.push_back(leftFrontBottom);

        // right cap (x1)
        unsigned int rightTopBack = (seg) * 2;
        unsigned int rightTopFront = rightTopBack + 1;
        unsigned int rightFrontBottom = frontStart + seg * 2 + 1;
        unsigned int rightBackBottom = backStart + seg * 2 + 1;
        inds.push_back(rightTopBack);
        inds.push_back(rightTopFront);
        inds.push_back(rightBackBottom);
        inds.push_back(rightBackBottom);
        inds.push_back(rightTopFront);
        inds.push_back(rightFrontBottom);

        Mesh mesh;
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ebo);
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        mesh.indexCount = static_cast<int>(inds.size());
        mesh.texture = 0;
        mesh.color = glm::vec3(0.2f, 0.5f, 0.95f);
        return mesh;
    }
}

WorldRepeater::WorldRepeater() {}
WorldRepeater::~WorldRepeater() { Cleanup(); }

void WorldRepeater::Configure(const std::string &mapPath, int repeats, float spd, float tSize)
{
    path = mapPath;
    segmentRepeats = repeats;
    speed = spd;
    tileSize = tSize;
}

void WorldRepeater::Cleanup()
{
    initialized = false;
}

void WorldRepeater::Update(Registry &registry, float dt)
{
    if (!initialized)
    {
        // load map file
        std::ifstream in(path);
        if (!in)
        {
            std::cerr << "WorldRepeater: failed to open map " << path << std::endl;
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

        float mapWidth = static_cast<float>(cols) * tileSize;
        float mapDepth = static_cast<float>(rows) * tileSize;

        this->mapWidth = mapWidth;
        this->mapDepth = mapDepth;

        Mesh groundMesh = CreatePlaneMesh(mapWidth, mapDepth, static_cast<float>(cols), static_cast<float>(rows));
        groundMesh.texture = Texture::Load("data/grass.jpg");

        Mesh cubeMesh = CreateCubeMesh();
        GLuint wood = Texture::Load("data/wood.jpg");
        if (wood)
            cubeMesh.texture = wood;

        // a flat blue tile
        Mesh blueMesh = CreatePlaneMesh(tileSize, tileSize, 1.0f, 1.0f);
        blueMesh.texture = 0;
        blueMesh.color = glm::vec3(0.2f, 0.2f, 0.9f);

        Mesh waterBaseMesh = CreatePlaneMesh(tileSize, tileSize, 1.0f, 1.0f);

        GLuint grassTex = Texture::Load("data/grass.jpg");
        if (grassTex)
            waterBaseMesh.texture = grassTex;
        else
            waterBaseMesh.color = glm::vec3(0.15f, 0.8f, 0.25f);
        Mesh waveMesh = CreateWaveMesh(tileSize, 28);
        GLuint waterTex = Texture::Load("data/water.jpg");
        if (waterTex)
            waveMesh.texture = waterTex;
        else
            waveMesh.color = glm::vec3(0.2f, 0.5f, 0.95f);

        // create repeated segments along +Z
        copies.clear();
        copies.reserve(segmentRepeats);
        copiesInitialPositions.clear();
        copiesInitialPositions.reserve(segmentRepeats);
        for (int s = 0; s < segmentRepeats; ++s)
        {
            float baseZ = static_cast<float>(s) * mapDepth;
            std::vector<Entity> created;
            std::vector<glm::vec3> initialPos;

            // create ground entity
            Entity g = registry.CreateEntity();
            Transform gt;
            gt.position = glm::vec3(0.0f, 0.0f, baseZ);
            gt.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            registry.AddComponent<Transform>(g, gt);
            registry.AddComponent<Mesh>(g, groundMesh);
            Collider groundCol;
            groundCol.type = Collider::AABB;
            groundCol.halfExtents = glm::vec3(mapWidth * 0.5f, 0.1f, mapDepth * 0.5f);
            registry.AddComponent<Collider>(g, groundCol);
            created.push_back(g);
            initialPos.push_back(gt.position);

            float offsetX = (static_cast<float>(cols - 1) * tileSize) * 0.5f;
            float offsetZ = (static_cast<float>(rows - 1) * tileSize) * 0.5f;
            for (size_t r = 0; r < rows; ++r)
            {
                for (size_t c = 0; c < cols; ++c)
                {
                    char ch = '0';
                    if (c < lines[r].size())
                        ch = lines[r][c];

                    if (ch == '1' || ch == '2')
                    {
                        float x = static_cast<float>(c) * tileSize - offsetX;
                        float z = static_cast<float>(r) * tileSize - offsetZ + baseZ;

                        if (ch == '1')
                        {
                            Entity e = registry.CreateEntity();
                            Transform t;
                            t.position = glm::vec3(x, tileSize * 1.0f, z);
                            t.scale = glm::vec3(tileSize, tileSize * 2.0f, tileSize);
                            registry.AddComponent<Transform>(e, t);
                            registry.AddComponent<Mesh>(e, cubeMesh);
                            Collider ccol;
                            ccol.type = Collider::AABB;
                            ccol.halfExtents = glm::vec3(tileSize * 0.5f, tileSize * 1.0f, tileSize * 0.5f);
                            registry.AddComponent<Collider>(e, ccol);
                            created.push_back(e);
                            initialPos.push_back(t.position);
                        }
                        else
                        {
                            Entity baseE = registry.CreateEntity();
                            Transform bt;
                            bt.position = glm::vec3(x, 0.01f, z);
                            bt.scale = glm::vec3(1.0f, 1.0f, 1.0f);
                            registry.AddComponent<Transform>(baseE, bt);
                            registry.AddComponent<Mesh>(baseE, waterBaseMesh);
                            created.push_back(baseE);
                            initialPos.push_back(bt.position);

                            // wave band on top (filled down to floor). place slightly above base to avoid z-fighting
                            Entity waveE = registry.CreateEntity();
                            Transform vt;
                            // place wave so its bottom sits near the floor to fill downwards
                            float waveYOffset = 0.02f;
                            vt.position = glm::vec3(x, waveYOffset, z);
                            vt.scale = glm::vec3(1.0f, 1.0f, 1.0f);
                            registry.AddComponent<Transform>(waveE, vt);
                            registry.AddComponent<Mesh>(waveE, waveMesh);
                            created.push_back(waveE);
                            initialPos.push_back(vt.position);
                        }
                    }
                }
            }

            copies.push_back(std::move(created));
            copiesInitialPositions.push_back(std::move(initialPos));
        }

        initialized = true;
        std::cerr << "WorldRepeater: initialized repeats=" << segmentRepeats << " mapDepth=" << mapDepth << std::endl;
    }

    // deterministic scrolling
    totalScroll -= static_cast<double>(speed) * static_cast<double>(dt);
    if (mapDepth <= 0.0f)
        return;

    double totalLength = static_cast<double>(mapDepth) * static_cast<double>(segmentRepeats);
    if (totalLength <= 0.0)
        return;

    // compute scroll offset in [0, totalLength)
    double scrollOffset = fmod(totalScroll, totalLength);
    if (scrollOffset < 0.0)
        scrollOffset += totalLength;

    // update each copy's entities based on scroll offset
    for (size_t i = 0; i < copies.size(); ++i)
    {
        if (copies[i].empty())
            continue;

        double baseZ = static_cast<double>(i) * static_cast<double>(mapDepth);
        double desiredGroundZ = baseZ - scrollOffset;

        if (desiredGroundZ < -totalLength * 0.5)
            desiredGroundZ += totalLength * std::ceil((-totalLength * 0.5 - desiredGroundZ) / totalLength);
        else if (desiredGroundZ >= totalLength * 0.5)
            desiredGroundZ -= totalLength * std::floor((desiredGroundZ - totalLength * 0.5) / totalLength + 1.0);

        double shift = desiredGroundZ - static_cast<double>(copiesInitialPositions[i][0].z);

        for (size_t j = 0; j < copies[i].size(); ++j)
        {
            Entity e = copies[i][j];
            Transform *t = registry.GetComponent<Transform>(e);
            if (!t)
                continue;
            glm::vec3 init = copiesInitialPositions[i][j];
            t->position.x = init.x;
            t->position.y = init.y;
            t->position.z = static_cast<float>(init.z + shift);
        }
    }
}
