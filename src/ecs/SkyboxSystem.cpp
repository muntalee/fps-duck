#include "ecs/SkyboxSystem.hpp"
#include <stb_image.h>
#include "ecs/Registry.hpp"
#include "ecs/Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>

// cube for drawing skybox (positions only)
static const float skyboxVerts[] = {
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f};

SkyboxSystem::SkyboxSystem()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerts), skyboxVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    // load skybox shader
    std::ifstream inv("data/shaders/skybox_vert.glsl");
    std::ifstream inf("data/shaders/skybox_frag.glsl");
    if (inv && inf)
    {
        std::ostringstream sv, sf;
        sv << inv.rdbuf();
        sf << inf.rdbuf();
        shader = new Shader(sv.str().c_str(), sf.str().c_str());
    }
    else
    {
        std::cerr << "SkyboxSystem: missing shaders data/shaders/skybox_*.glsl" << std::endl;
    }
}

SkyboxSystem::~SkyboxSystem()
{
    Cleanup();
}

// helper: copy a rectangular region from src (w x h, channels) into dst (fw x fh)
static void CopyRegion(const unsigned char *src, int srcW, int srcH, int channels,
                       int col, int row, int cols, int rows,
                       unsigned char *dst, int fw, int fh)
{
    int faceW = fw;
    int faceH = fh;
    int srcFaceW = srcW / cols;
    int srcFaceH = srcH / rows;
    int startX = col * srcFaceW;
    int startY = row * srcFaceH;

    for (int y = 0; y < srcFaceH; ++y)
    {
        for (int x = 0; x < srcFaceW; ++x)
        {
            int sx = startX + x;
            int sy = startY + y;
            int sidx = (sy * srcW + sx) * channels;
            int didx = (y * srcFaceW + x) * channels;
            for (int c = 0; c < channels; ++c)
                dst[didx + c] = src[sidx + c];
        }
    }
}

bool SkyboxSystem::LoadFromCrossImage(const std::string &path)
{
    int width, height, channels;
    // don't flip -- we want rows as in file
    stbi_set_flip_vertically_on_load(0);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "SkyboxSystem: failed to load " << path << std::endl;
        return false;
    }

    // expect 4x3 grid
    const int cols = 4;
    const int rows = 3;
    if (width % cols != 0 || height % rows != 0)
    {
        std::cerr << "SkyboxSystem: image size not divisible by 4x3 grid: " << width << "x" << height << std::endl;
        stbi_image_free(data);
        return false;
    }

    faceW = width / cols;
    faceH = height / rows;

    // allocate and create cubemap
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    // mapping for this cross image layout:
    std::pair<int, int> faceCoords[6] = {
        {2, 1}, // +X (right)
        {0, 1}, // -X (left)
        {1, 0}, // +Y (top)
        {1, 2}, // -Y (bottom)
        {1, 1}, // +Z (front)
        {3, 1}  // -Z (back)
    };

    int faceSize = faceW * faceH * channels;
    std::vector<unsigned char> faceBuf(faceSize);

    for (int i = 0; i < 6; ++i)
    {
        int col = faceCoords[i].first;
        int row = faceCoords[i].second;
        CopyRegion(data, width, height, channels, col, row, cols, rows, faceBuf.data(), faceW, faceH);

        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        // upload to cubemap face
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, faceW, faceH, 0, format, GL_UNSIGNED_BYTE, faceBuf.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    stbi_image_free(data);
    std::cerr << "SkyboxSystem: loaded cross image " << path << " (" << width << "x" << height << ") -> face " << faceW << "x" << faceH << std::endl;
    return true;
}

void SkyboxSystem::Update(Registry &registry, float dt)
{
    if (!shader || shader->id == 0 || cubemap == 0)
        return;

    // get camera view matrix
    glm::mat4 view(1.0f);
    for (auto [e, cam] : registry.View<Camera>())
    {
        view = cam->GetViewMatrix();
        break;
    }

    // projection: match RenderSystem behavior via ImGui IO
    ImGuiIO &io = ImGui::GetIO();
    float width = io.DisplaySize.x > 0.0f ? io.DisplaySize.x : 800.0f;
    float height = io.DisplaySize.y > 0.0f ? io.DisplaySize.y : 600.0f;
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    shader->Use();
    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
    shader->SetMat4("view", &viewNoTrans[0][0]);
    shader->SetMat4("proj", &proj[0][0]);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    GLint loc = glGetUniformLocation(shader->id, "skybox");
    if (loc >= 0)
        glUniform1i(loc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void SkyboxSystem::Cleanup()
{
    if (shader)
    {
        delete shader;
        shader = nullptr;
    }
    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (cubemap)
    {
        glDeleteTextures(1, &cubemap);
        cubemap = 0;
    }
}
