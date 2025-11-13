#include "ecs/Model.hpp"
#include "ecs/Texture.hpp"
#include <glm/glm.hpp>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

struct ObjVertex
{
    int vp = -1, vt = -1, vn = -1;
};

static std::string Trim(const std::string &s)
{
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos)
        return {};
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

Mesh Model::LoadFromOBJ(const std::string &path)
{
    Mesh mesh;
    std::ifstream in(path);
    if (!in)
    {
        std::cerr << "Model::LoadFromOBJ - failed to open: " << path << std::endl;
        return mesh;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    std::vector<unsigned int> indices;
    std::vector<float> vertexData; // interleaved pos(3) normal(3) uv(2)

    std::unordered_map<std::string, unsigned int> uniqueVerts;

    fs::path objPath(path);
    fs::path baseDir = objPath.parent_path();
    std::string mtlFile;

    std::string line;
    while (std::getline(in, line))
    {
        line = Trim(line);
        if (line.rfind("v ", 0) == 0)
        {
            std::istringstream ss(line.substr(2));
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (line.rfind("vt ", 0) == 0)
        {
            std::istringstream ss(line.substr(3));
            glm::vec2 t;
            ss >> t.x >> t.y;
            uvs.push_back(t);
        }
        else if (line.rfind("vn ", 0) == 0)
        {
            std::istringstream ss(line.substr(3));
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (line.rfind("f ", 0) == 0)
        {
            std::istringstream ss(line.substr(2));
            std::string a, b, c;
            // support polygons by triangulating (assume triangles or quads)
            std::vector<std::string> verts;
            std::string token;
            while (ss >> token)
                verts.push_back(token);
            if (verts.size() < 3)
                continue;

            auto processFace = [&](const std::string &fv) -> unsigned int
            {
                auto it = uniqueVerts.find(fv);
                if (it != uniqueVerts.end())
                    return it->second;

                // parse indices
                ObjVertex ov;
                size_t p1 = fv.find('/');
                if (p1 == std::string::npos)
                {
                    ov.vp = std::stoi(fv) - 1;
                }
                else
                {
                    std::string s1 = fv.substr(0, p1);
                    ov.vp = s1.empty() ? -1 : (std::stoi(s1) - 1);
                    size_t p2 = fv.find('/', p1 + 1);
                    if (p2 == std::string::npos)
                    {
                        std::string s2 = fv.substr(p1 + 1);
                        ov.vt = s2.empty() ? -1 : (std::stoi(s2) - 1);
                    }
                    else
                    {
                        std::string s2 = fv.substr(p1 + 1, p2 - p1 - 1);
                        ov.vt = s2.empty() ? -1 : (std::stoi(s2) - 1);
                        std::string s3 = fv.substr(p2 + 1);
                        ov.vn = s3.empty() ? -1 : (std::stoi(s3) - 1);
                    }
                }

                glm::vec3 p(0.0f);
                glm::vec2 t(0.0f);
                glm::vec3 n(0.0f);
                if (ov.vp >= 0 && ov.vp < (int)positions.size())
                    p = positions[ov.vp];
                if (ov.vt >= 0 && ov.vt < (int)uvs.size())
                    t = uvs[ov.vt];
                if (ov.vn >= 0 && ov.vn < (int)normals.size())
                    n = normals[ov.vn];

                unsigned int newIndex = (unsigned int)(vertexData.size() / 8);
                uniqueVerts[fv] = newIndex;

                // push pos, normal, uv
                vertexData.push_back(p.x);
                vertexData.push_back(p.y);
                vertexData.push_back(p.z);
                vertexData.push_back(n.x);
                vertexData.push_back(n.y);
                vertexData.push_back(n.z);
                vertexData.push_back(t.x);
                vertexData.push_back(t.y);

                return newIndex;
            };

            // triangulate fan
            unsigned int i0 = processFace(verts[0]);
            for (size_t i = 1; i + 1 < verts.size(); ++i)
            {
                unsigned int i1 = processFace(verts[i]);
                unsigned int i2 = processFace(verts[i + 1]);
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);
            }
        }
        else if (line.rfind("mtllib ", 0) == 0)
        {
            mtlFile = Trim(line.substr(7));
        }
    }

    // try load diffuse texture from mtl if present
    GLuint tex = 0;
    if (!mtlFile.empty())
    {
        fs::path mtlPath = baseDir / mtlFile;
        if (fs::exists(mtlPath))
        {
            std::ifstream m(mtlPath);
            std::string l;
            while (std::getline(m, l))
            {
                l = Trim(l);
                if (l.rfind("map_Kd ", 0) == 0)
                {
                    std::string texRel = Trim(l.substr(7));
                    fs::path texPath = baseDir / texRel;
                    // normalize and try variants
                    texPath = fs::weakly_canonical(texPath);
                    if (fs::exists(texPath))
                        tex = Texture::Load(texPath.string());
                    else
                    {
                        // try same directory with filename only
                        fs::path candidate = baseDir / fs::path(texRel).filename();
                        if (fs::exists(candidate))
                            tex = Texture::Load(candidate.string());
                    }
                    if (tex)
                        break;
                }
            }
        }
    }

    // fallback cuz yes
    if (!tex)
    {
        for (auto &p : fs::directory_iterator(baseDir))
        {
            auto ext = p.path().extension().string();
            for (auto &c : ext)
                c = (char)tolower(c);
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png")
            {
                tex = Texture::Load(p.path().string());
                if (tex)
                    break;
            }
        }
    }

    // create GL buffers if we have vertex data
    if (!vertexData.empty() && !indices.empty())
    {
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ebo);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texcoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        mesh.indexCount = (int)indices.size();
        mesh.texture = tex;
    }

    return mesh;
}
