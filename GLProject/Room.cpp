#include "Room.h"
#include "tools/TextureLoader.h"
#include "tools/Box.h"
#include <learnopengl/model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

static std::vector<GLuint> roomTextures;
static std::vector<PolygonWithTexture*> roomParts;

static std::vector<GLuint> tableTextures;
static std::vector<Box*> tableParts;

static std::vector<GLuint> zoneTexture;
static std::vector<PolygonWithTexture*> zoneParts;

static std::vector<GLuint> posterTexture;
static PolygonWithTexture* posterPart = nullptr;
static PolygonWithTexture* posterShadow = nullptr;

static std::vector<GLuint> decorTextures;
static std::vector<Box*> decorParts;
static PolygonWithTexture* chessBoard = nullptr;
static std::vector<GLuint> chestTexture;
static Model* decorModel = nullptr;
static glm::mat4 decorModelMatrix(1.0f);

static std::vector<GLuint> rugTexture;
static PolygonWithTexture* rugPart = nullptr;
static std::vector<Box*> chairParts;
static std::vector<Box*> radioParts;
static std::vector<Box*> lanternParts;
static std::vector<Box*> glowParts;

static std::vector<GLuint> leafTexture;
static std::vector<Box*> plantParts;
static std::vector<PolygonWithTexture*> plantLeaves;

static const float ROOM = 15.0f;
static const float FLOOR_Y = -3.0f;
static const float CEIL_Y = 9.0f;

static void addQuad(const std::vector<glm::vec3>& corners, const std::vector<glm::vec2>& uvs, GLuint& texture)
{
    PolygonWithTexture* quad = new PolygonWithTexture(corners, uvs);
    quad->setTexture(texture);
    roomParts.push_back(quad);
}

void buildRoom()
{
    std::vector<std::string> paths = { "textures/floor.jpg", "textures/wall.jpg" };
    roomTextures = loadTextures(paths, GL_REPEAT, GL_LINEAR);
    if (roomTextures.size() != paths.size())
    {
        std::cerr << "Room: expected " << paths.size() << " textures, got " << roomTextures.size() << std::endl;
        return;
    }

    addQuad(
        {
            glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y, ROOM),
            glm::vec3(ROOM, FLOOR_Y, ROOM), glm::vec3(ROOM, FLOOR_Y, -ROOM)
        },
        { glm::vec2(0.0f, 10.0f), glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 0.0f), glm::vec2(10.0f, 10.0f) },
        roomTextures[0]);

    std::vector<glm::vec2> wallUV =
        { glm::vec2(0.0f, 2.0f), glm::vec2(0.0f, 0.0f), glm::vec2(5.0f, 0.0f), glm::vec2(5.0f, 2.0f) };

    addQuad(
        { glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y, -ROOM),
          glm::vec3(ROOM, FLOOR_Y, -ROOM), glm::vec3(ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad(
        { glm::vec3(ROOM, CEIL_Y, ROOM), glm::vec3(ROOM, FLOOR_Y, ROOM),
          glm::vec3(-ROOM, FLOOR_Y, ROOM), glm::vec3(-ROOM, CEIL_Y, ROOM) }, wallUV, roomTextures[1]);
    addQuad(
        { glm::vec3(-ROOM, CEIL_Y, ROOM), glm::vec3(-ROOM, FLOOR_Y, ROOM),
          glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad(
        { glm::vec3(ROOM, CEIL_Y, -ROOM), glm::vec3(ROOM, FLOOR_Y, -ROOM),
          glm::vec3(ROOM, FLOOR_Y, ROOM), glm::vec3(ROOM, CEIL_Y, ROOM) }, wallUV, roomTextures[1]);

    addQuad(
        {
            glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3(ROOM, CEIL_Y, -ROOM),
            glm::vec3(ROOM, CEIL_Y, ROOM), glm::vec3(-ROOM, CEIL_Y, ROOM)
        },
        { glm::vec2(0.0f, 5.0f), glm::vec2(5.0f, 5.0f), glm::vec2(5.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        roomTextures[1]);

    std::vector<std::string> tablePaths = { "textures/table_felt.jpg", "textures/table_wood.jpg" };
    tableTextures = loadTextures(tablePaths, GL_REPEAT, GL_LINEAR);
    if (tableTextures.size() != tablePaths.size())
    {
        std::cerr << "Room: table textures failed to load" << std::endl;
        return;
    }

    auto addBox = [](glm::vec3 center, glm::vec3 size, int tex)
    {
        tableParts.push_back(new Box(center, size, tableTextures[tex]));
    };
    const int FELT = 0, WOOD = 1;

    addBox(glm::vec3(0.0f, -0.15f, 0.0f), glm::vec3(12.0f, 0.3f, 8.0f), FELT);

    addBox(glm::vec3(0.0f, -0.12f, 4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);
    addBox(glm::vec3(0.0f, -0.12f, -4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);
    addBox(glm::vec3(6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);
    addBox(glm::vec3(-6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);

    addBox(glm::vec3(0.0f, -0.6f, 4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3(0.0f, -0.6f, -4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3(6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);
    addBox(glm::vec3(-6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);

    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
        {
            float x = sx * 6.0f, z = sz * 4.0f;
            addBox(glm::vec3(x, -1.65f, z), glm::vec3(0.5f, 2.7f, 0.5f), WOOD);
            addBox(glm::vec3(x, -0.85f, z), glm::vec3(0.7f, 0.3f, 0.7f), WOOD);
            addBox(glm::vec3(x, -2.85f, z), glm::vec3(0.7f, 0.3f, 0.7f), WOOD);
        }

    zoneTexture = loadTextures({ "textures/white.jpg" }, GL_REPEAT, GL_LINEAR);
    if (zoneTexture.empty())
    {
        std::cerr << "Room: white texture failed to load" << std::endl;
        return;
    }
    const float LINE_Y = 0.005f;

    auto addBar = [&](float x0, float x1, float z0, float z1, float y, glm::vec3 color)
    {
        PolygonWithTexture* bar = new PolygonWithTexture(
            { glm::vec3(x0, y, z1), glm::vec3(x1, y, z1), glm::vec3(x1, y, z0), glm::vec3(x0, y, z0) },
            { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
            color);
        bar->setTexture(zoneTexture[0]);
        zoneParts.push_back(bar);
    };

    auto addOutline = [&](float xMin, float xMax, float zMin, float zMax, float t, glm::vec3 color)
    {
        addBar(xMin, xMax, zMax - t, zMax, LINE_Y, color);
        addBar(xMin, xMax, zMin, zMin + t, LINE_Y, color);
        addBar(xMin, xMin + t, zMin + t, zMax - t, LINE_Y, color);
        addBar(xMax - t, xMax, zMin + t, zMax - t, LINE_Y, color);
    };

    auto addZone = [&](float xMin, float xMax, float zMin, float zMax, glm::vec3 color)
    {
        addOutline(xMin, xMax, zMin, zMax, 0.04f, color);
        addOutline(xMin + 0.08f, xMax - 0.08f, zMin + 0.08f, zMax - 0.08f, 0.015f, color);
        const float c = 0.16f, y = LINE_Y + 0.0015f;
        addBar(xMin, xMin + c, zMin, zMin + c, y, color);
        addBar(xMax - c, xMax, zMin, zMin + c, y, color);
        addBar(xMin, xMin + c, zMax - c, zMax, y, color);
        addBar(xMax - c, xMax, zMax - c, zMax, y, color);
    };

    auto addSlots = [&](float centerZ, glm::vec3 color)
    {
        for (int i = 0; i < 5; i++)
        {
            float cx = -2.6f + 1.3f * i;
            addOutline(cx - 0.55f, cx + 0.55f, centerZ - 0.8f, centerZ + 0.8f, 0.02f, color);
        }
    };

    const glm::vec3 GOLD(0.85f, 0.70f, 0.25f);
    const glm::vec3 DIM_GOLD(0.5f, 0.4f, 0.18f);
    const glm::vec3 SLOT(0.6f, 0.5f, 0.22f);

    addZone(-3.3f, 3.3f, 1.0f, 3.0f, GOLD);
    addSlots(2.0f, SLOT);
    addZone(3.42f, 4.58f, 1.0f, 3.0f, GOLD);
    addZone(-3.3f, 3.3f, -3.0f, -1.0f, DIM_GOLD);
    addSlots(-2.0f, DIM_GOLD * 0.8f);

    rugTexture = loadTextures({ "textures/rug.jpg" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (rugTexture.empty())
    {
        std::cerr << "Room: rug texture failed to load" << std::endl;
        return;
    }
    const float RUG_Y = FLOOR_Y + 0.005f;
    rugPart = new PolygonWithTexture(
        { glm::vec3(-10.0f, RUG_Y, 6.5f), glm::vec3(10.0f, RUG_Y, 6.5f),
          glm::vec3(10.0f, RUG_Y, -6.5f), glm::vec3(-10.0f, RUG_Y, -6.5f) },
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) });
    rugPart->setTexture(rugTexture[0]);

    auto addChair = [&](glm::vec3 origin, float rotY, int wood, int cloth)
    {
        const float SCALE = 1.3f;
        const float c = glm::cos(glm::radians(rotY));
        const float s = glm::sin(glm::radians(rotY));

        auto place = [&](glm::vec3 offset, glm::vec3 size, int tex)
        {
            offset *= SCALE;
            size *= SCALE;

            glm::vec3 center = origin + glm::vec3(offset.x * c + offset.z * s, offset.y,
                                                  -offset.x * s + offset.z * c);
            glm::vec3 worldSize(std::fabs(c) * size.x + std::fabs(s) * size.z, size.y,
                                std::fabs(s) * size.x + std::fabs(c) * size.z);
            chairParts.push_back(new Box(center, worldSize, tableTextures[tex]));
        };

        place(glm::vec3(-0.55f, 0.75f, 0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(0.55f, 0.75f, 0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(-0.55f, 0.75f, -0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(0.55f, 0.75f, -0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(0.0f, 1.58f, 0.0f), glm::vec3(1.35f, 0.16f, 1.35f), wood);
        place(glm::vec3(0.0f, 1.70f, 0.0f), glm::vec3(1.05f, 0.10f, 1.00f), cloth);

        place(glm::vec3(-0.52f, 2.54f, -0.60f), glm::vec3(0.13f, 2.12f, 0.16f), wood);
        place(glm::vec3(0.52f, 2.54f, -0.60f), glm::vec3(0.13f, 2.12f, 0.16f), wood);
        place(glm::vec3(0.0f, 1.48f, -0.60f), glm::vec3(1.15f, 0.13f, 0.17f), wood);
        place(glm::vec3(0.0f, 3.62f, -0.60f), glm::vec3(1.38f, 0.15f, 0.19f), wood);
        for (int i = -1; i <= 1; i++)
            place(glm::vec3(0.30f * i, 2.55f, -0.60f), glm::vec3(0.07f, 1.90f, 0.12f), wood);

        for (int side = -1; side <= 1; side += 2)
        {
            place(glm::vec3(0.68f * side, 1.91f, 0.38f), glm::vec3(0.12f, 0.58f, 0.12f), wood);
            place(glm::vec3(0.68f * side, 1.91f, -0.42f), glm::vec3(0.12f, 0.58f, 0.12f), wood);
            place(glm::vec3(0.68f * side, 2.22f, -0.02f), glm::vec3(0.16f, 0.12f, 1.02f), wood);
        }
    };

    addChair(glm::vec3(-7.7f, FLOOR_Y, 0.0f), 90.0f, WOOD, FELT);
    addChair(glm::vec3(7.7f, FLOOR_Y, 0.0f), -90.0f, WOOD, FELT);

    const float RADIO_RIGHT_SHIFT = 5.5f;
    auto addRadioPart = [&](glm::vec3 center, glm::vec3 size, GLuint texture, glm::vec3 tint)
    {
        center.x += RADIO_RIGHT_SHIFT;
        radioParts.push_back(new Box(center, size, texture, tint));
    };

    const glm::vec3 RADIO_WOOD(0.62f, 0.46f, 0.27f);
    const glm::vec3 RADIO_DARK(0.12f, 0.10f, 0.075f);
    const glm::vec3 RADIO_BRASS(0.72f, 0.50f, 0.22f);

    addRadioPart(glm::vec3(5.00f, -1.43f, -7.50f),
                 glm::vec3(3.50f, 0.18f, 1.90f), tableTextures[WOOD], RADIO_WOOD);

    for (float x : {3.45f, 6.55f})
        for (float z : {-8.22f, -6.78f})
            addRadioPart(glm::vec3(x, -2.18f, z),
                         glm::vec3(0.14f, 1.55f, 0.14f), tableTextures[WOOD], RADIO_WOOD);

    addRadioPart(glm::vec3(4.25f, -0.78f, -7.53f),
                 glm::vec3(1.70f, 1.10f, 0.90f), tableTextures[WOOD], RADIO_WOOD);

    addRadioPart(glm::vec3(4.25f, -0.78f, -7.065f),
                 glm::vec3(1.54f, 0.94f, 0.045f), zoneTexture[0], RADIO_DARK);

    for (int i = 0; i < 6; i++)
    {
        float y = -1.10f + 0.12f * i;
        addRadioPart(glm::vec3(3.88f, y, -7.035f),
                     glm::vec3(0.40f, 0.022f, 0.025f), zoneTexture[0], RADIO_BRASS);
    }

    addRadioPart(glm::vec3(4.62f, -0.72f, -7.035f),
                 glm::vec3(0.42f, 0.34f, 0.025f), zoneTexture[0], RADIO_DARK);
    glowParts.push_back(new Box(glm::vec3(4.62f + RADIO_RIGHT_SHIFT, -0.72f, -7.015f),
                                glm::vec3(0.34f, 0.25f, 0.018f), zoneTexture[0],
                                glm::vec3(0.38f, 0.18f, 0.045f)));

    addRadioPart(glm::vec3(4.90f, -1.10f, -7.025f),
                 glm::vec3(0.12f, 0.12f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(4.90f, -0.42f, -7.025f),
                 glm::vec3(0.12f, 0.12f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(4.82f, -0.03f, -7.80f),
                 glm::vec3(0.025f, 0.62f, 0.025f), zoneTexture[0], RADIO_BRASS);

    addRadioPart(glm::vec3(3.15f, -1.27f, -6.78f),
                 glm::vec3(0.34f, 0.07f, 0.34f), tableTextures[WOOD], RADIO_WOOD);
    addRadioPart(glm::vec3(3.15f, -1.03f, -6.78f),
                 glm::vec3(0.055f, 0.42f, 0.055f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(3.15f, -0.78f, -6.78f),
                 glm::vec3(0.28f, 0.25f, 0.28f), zoneTexture[0], RADIO_BRASS);
    glowParts.push_back(new Box(glm::vec3(7.15f, -0.78f, -6.74f),
                                glm::vec3(0.18f, 0.15f, 0.18f), zoneTexture[0],
                                glm::vec3(0.32f, 0.14f, 0.035f)));

    addRadioPart(glm::vec3(6.05f, -1.24f, -7.50f),
                 glm::vec3(0.66f, 0.14f, 0.62f), tableTextures[WOOD], RADIO_WOOD);
    addRadioPart(glm::vec3(6.05f, -0.87f, -7.50f),
                 glm::vec3(0.12f, 0.62f, 0.12f), zoneTexture[0], RADIO_BRASS);

    const float FAN_FRONT_Z = -7.03f;

    addRadioPart(glm::vec3(6.05f, -0.43f, FAN_FRONT_Z),
                 glm::vec3(0.82f, 0.075f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(6.05f, -1.15f, FAN_FRONT_Z),
                 glm::vec3(0.82f, 0.075f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(5.69f, -0.79f, FAN_FRONT_Z),
                 glm::vec3(0.075f, 0.72f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(6.41f, -0.79f, FAN_FRONT_Z),
                 glm::vec3(0.075f, 0.72f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(6.05f, -0.79f, FAN_FRONT_Z + 0.085f),
                 glm::vec3(0.15f, 0.15f, 0.07f), zoneTexture[0], RADIO_BRASS);

    const glm::vec3 BRASS(0.55f, 0.45f, 0.30f);
    for (float lx : { -4.0f, 4.0f })
    {
        lanternParts.push_back(new Box(glm::vec3(lx, 6.60f, 0.0f), glm::vec3(0.06f, 4.80f, 0.06f), zoneTexture[0], BRASS));
        lanternParts.push_back(new Box(glm::vec3(lx, 4.15f, 0.0f), glm::vec3(0.86f, 0.18f, 0.86f), zoneTexture[0], BRASS));
        for (int sx = -1; sx <= 1; sx += 2)
            for (int sz = -1; sz <= 1; sz += 2)
                lanternParts.push_back(new Box(glm::vec3(lx + sx * 0.30f, 3.60f, sz * 0.30f),
                                               glm::vec3(0.09f, 1.00f, 0.09f), zoneTexture[0], BRASS));
        lanternParts.push_back(new Box(glm::vec3(lx, 3.05f, 0.0f), glm::vec3(0.80f, 0.14f, 0.80f), zoneTexture[0], BRASS));
        glowParts.push_back(new Box(glm::vec3(lx, 3.60f, 0.0f), glm::vec3(0.60f, 0.90f, 0.60f), zoneTexture[0],
                                    glm::vec3(1.0f, 0.80f, 0.48f)));
    }

    for (float sx : { -5.6f, 5.6f })
    {
        const float SZ = -ROOM + 0.55f;
        lanternParts.push_back(new Box(glm::vec3(sx, 2.30f, -ROOM + 0.20f), glm::vec3(0.18f, 0.22f, 0.60f), zoneTexture[0], BRASS));
        lanternParts.push_back(new Box(glm::vec3(sx, 2.82f, SZ), glm::vec3(0.88f, 0.16f, 0.88f), zoneTexture[0], BRASS));
        lanternParts.push_back(new Box(glm::vec3(sx, 1.90f, SZ), glm::vec3(0.80f, 0.14f, 0.80f), zoneTexture[0], BRASS));
        for (int px = -1; px <= 1; px += 2)
            for (int pz = -1; pz <= 1; pz += 2)
                lanternParts.push_back(new Box(glm::vec3(sx + px * 0.33f, 2.35f, SZ + pz * 0.33f),
                                               glm::vec3(0.08f, 0.80f, 0.08f), zoneTexture[0], BRASS));
        glowParts.push_back(new Box(glm::vec3(sx, 2.35f, SZ), glm::vec3(0.60f, 0.72f, 0.60f), zoneTexture[0],
                                    glm::vec3(1.0f, 0.86f, 0.58f)));
    }

    leafTexture = loadTextures({ "textures/leaf.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (leafTexture.empty())
    {
        std::cerr << "Room: leaf texture failed to load" << std::endl;
        return;
    }
    const float PLX = -11.5f, PLZ = -6.6f;
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.06f, PLZ), glm::vec3(0.62f, 0.12f, 0.62f), zoneTexture[0], BRASS));
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.40f, PLZ), glm::vec3(0.80f, 0.55f, 0.80f), zoneTexture[0], BRASS));
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.73f, PLZ), glm::vec3(0.92f, 0.12f, 0.92f), zoneTexture[0], BRASS));
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.79f, PLZ), glm::vec3(0.78f, 0.06f, 0.78f), zoneTexture[0],
                                 glm::vec3(0.24f, 0.18f, 0.12f)));
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 1.14f, PLZ), glm::vec3(0.18f, 0.70f, 0.18f), tableTextures[WOOD]));

    const glm::vec3 CROWN(PLX, FLOOR_Y + 1.50f, PLZ);
    auto addFrond = [&](float angleDeg, float tiltDeg, float len, float halfWidth)
    {
        const float a = glm::radians(angleDeg), t = glm::radians(tiltDeg);
        glm::vec3 dir(std::cos(a) * std::cos(t), std::sin(t), std::sin(a) * std::cos(t));
        glm::vec3 side = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
        if (glm::length(side) < 0.01f)
            side = glm::vec3(1.0f, 0.0f, 0.0f);
        side = glm::normalize(side);
        glm::vec3 base = CROWN + dir * 0.06f;
        glm::vec3 tip = base + dir * len;
        tip.y -= len * 0.25f;
        plantLeaves.push_back(new PolygonWithTexture(
            { base - side * halfWidth, base + side * halfWidth,
              tip + side * halfWidth * 0.80f, tip - side * halfWidth * 0.80f },
            { glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
            glm::vec3(1.35f, 1.55f, 1.25f)));
        plantLeaves.back()->setTexture(leafTexture[0]);
    };
    for (int i = 0; i < 12; i++)
        addFrond(30.0f * i, 18.0f + (i % 3) * 14.0f, 1.80f + (i % 4) * 0.16f, 0.46f);
    for (int i = 0; i < 5; i++)
        addFrond(72.0f * i, 58.0f, 1.30f, 0.36f);

    posterTexture = loadTextures({ "textures/poster.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (posterTexture.empty())
    {
        std::cerr << "Room: poster texture failed to load" << std::endl;
        return;
    }
    const float PW = 8.0f * 2.0f / 3.0f / 2.0f;
    const float PB = -1.0f, PT = 7.0f;
    const float PZ = -ROOM + 0.02f;
    posterPart = new PolygonWithTexture(
        { glm::vec3(-PW, PT, PZ), glm::vec3(-PW, PB, PZ), glm::vec3(PW, PB, PZ), glm::vec3(PW, PT, PZ) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f) });
    posterPart->setTexture(posterTexture[0]);

    posterShadow = new PolygonWithTexture(
        { glm::vec3(-PW + 0.18f, PT - 0.18f, PZ - 0.01f), glm::vec3(-PW + 0.18f, PB - 0.18f, PZ - 0.01f),
          glm::vec3(PW + 0.18f, PB - 0.18f, PZ - 0.01f), glm::vec3(PW + 0.18f, PT - 0.18f, PZ - 0.01f) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
        glm::vec3(0.12f, 0.09f, 0.06f));
    posterShadow->setTexture(posterTexture[0]);

    decorTextures = loadTextures({ "textures/decor_chessboard.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (decorTextures.empty())
    {
        std::cerr << "Room: chessboard texture failed to load" << std::endl;
        return;
    }
    const float CX = -10.0f, CZ = -10.0f;
    const float LEG_H = 1.5f;
    const float TOP_Y = FLOOR_Y + LEG_H + 0.1f;
    auto addDecor = [&](glm::vec3 center, glm::vec3 size)
    {
        decorParts.push_back(new Box(center, size, tableTextures[1]));
    };
    addDecor(glm::vec3(CX, TOP_Y, CZ), glm::vec3(3.4f, 0.2f, 3.4f));
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
            addDecor(glm::vec3(CX + sx * 1.5f, FLOOR_Y + LEG_H / 2.0f, CZ + sz * 1.5f), glm::vec3(0.25f, LEG_H, 0.25f));
    addDecor(glm::vec3(CX, FLOOR_Y + 0.45f, CZ + 2.6f), glm::vec3(1.0f, 0.9f, 1.0f));
    addDecor(glm::vec3(CX, FLOOR_Y + 0.45f, CZ - 2.6f), glm::vec3(1.0f, 0.9f, 1.0f));

    const float BY = TOP_Y + 0.1f + 0.005f, B = 1.5f;
    chessBoard = new PolygonWithTexture(
        { glm::vec3(CX - B, BY, CZ + B), glm::vec3(CX + B, BY, CZ + B), glm::vec3(CX + B, BY, CZ - B), glm::vec3(CX - B, BY, CZ - B) },
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) });
    chessBoard->setTexture(decorTextures[0]);

    chestTexture = loadTextures({ "textures/decor_chest.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (chestTexture.empty())
    {
        std::cerr << "Room: chest texture failed to load" << std::endl;
        return;
    }

    decorParts.push_back(new Box(glm::vec3(10.0f, FLOOR_Y + 0.5f, -10.0f), glm::vec3(2.4f, 1.0f, 1.6f), chestTexture[0]));
    decorParts.push_back(new Box(glm::vec3(10.0f, FLOOR_Y + 1.25f, -10.0f), glm::vec3(2.5f, 0.5f, 1.7f), chestTexture[0]));

    decorModel = new Model("models/alrihla.glb");
    if (decorModel->meshes.empty())
    {
        std::cerr << "Room: 3D model failed to load" << std::endl;
        delete decorModel;
        decorModel = nullptr;
        return;
    }
    glm::vec3 mn(1e9f), mx(-1e9f);
    for (const Mesh& mesh : decorModel->meshes)
        for (const Vertex& v : mesh.vertices)
        {
            mn = glm::min(mn, v.Position);
            mx = glm::max(mx, v.Position);
        }
    glm::vec3 size = mx - mn;
    float s = 3.0f / glm::max(size.x, glm::max(size.y, size.z));
    glm::vec3 bottomCenter((mn.x + mx.x) * 0.5f, mn.y, (mn.z + mx.z) * 0.5f);
    decorModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-12.0f, FLOOR_Y, 8.0f))
                     * glm::scale(glm::mat4(1.0f), glm::vec3(s))
                     * glm::translate(glm::mat4(1.0f), -bottomCenter);
}

void drawRoom(Shader& shader)
{
    shader.setInt("textureSample", 0);
    glActiveTexture(GL_TEXTURE0);

    for (size_t i = 0; i + 1 < roomParts.size(); i++)
        roomParts[i]->draw(shader);

    if (rugPart)
        rugPart->draw(shader);

    for (Box* part : tableParts)
        part->draw(shader);

    for (Box* part : chairParts)
        part->draw(shader);

    for (Box* part : radioParts)
        part->draw(shader);

    for (Box* part : plantParts)
        part->draw(shader);
    for (PolygonWithTexture* leaf : plantLeaves)
        leaf->draw(shader);

    for (PolygonWithTexture* bar : zoneParts)
        bar->draw(shader);

    for (Box* part : decorParts)
        part->draw(shader);
    if (chessBoard)
        chessBoard->draw(shader);

    if (decorModel)
    {
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setMat4("model", decorModelMatrix);
        decorModel->Draw(shader);
        glActiveTexture(GL_TEXTURE0);
    }

    if (posterShadow)
        posterShadow->draw(shader);
    if (posterPart)
        posterPart->draw(shader);

    for (Box* part : lanternParts)
        part->draw(shader);
    shader.setFloat("emissive", 1.0f);
    for (Box* part : glowParts)
        part->draw(shader);
    shader.setFloat("emissive", 0.0f);

    if (!roomParts.empty())
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        roomParts.back()->draw(shader);
        glDisable(GL_CULL_FACE);
    }
}

void destroyRoom()
{
    for (PolygonWithTexture* part : roomParts)
    {
        part->deleteBuffers();
        delete part;
    }
    roomParts.clear();

    for (Box* part : tableParts)
    {
        part->destroy();
        delete part;
    }
    tableParts.clear();

    for (PolygonWithTexture* bar : zoneParts)
    {
        bar->deleteBuffers();
        delete bar;
    }
    zoneParts.clear();

    for (Box* part : decorParts)
    {
        part->destroy();
        delete part;
    }
    decorParts.clear();

    if (chessBoard)
    {
        chessBoard->deleteBuffers();
        delete chessBoard;
        chessBoard = nullptr;
    }

    glDeleteTextures(static_cast<GLsizei>(decorTextures.size()), decorTextures.data());
    glDeleteTextures(static_cast<GLsizei>(chestTexture.size()), chestTexture.data());
    glDeleteTextures(static_cast<GLsizei>(rugTexture.size()), rugTexture.data());
    glDeleteTextures(static_cast<GLsizei>(leafTexture.size()), leafTexture.data());

    for (PolygonWithTexture* leaf : plantLeaves)
    {
        leaf->deleteBuffers();
        delete leaf;
    }
    plantLeaves.clear();

    for (Box* part : plantParts)
    {
        part->destroy();
        delete part;
    }
    plantParts.clear();

    if (rugPart)
    {
        rugPart->deleteBuffers();
        delete rugPart;
        rugPart = nullptr;
    }

    for (Box* part : chairParts)
    {
        part->destroy();
        delete part;
    }
    chairParts.clear();

    for (Box* part : radioParts)
    {
        part->destroy();
        delete part;
    }
    radioParts.clear();

    for (Box* part : lanternParts)
    {
        part->destroy();
        delete part;
    }
    lanternParts.clear();

    for (Box* part : glowParts)
    {
        part->destroy();
        delete part;
    }
    glowParts.clear();

    delete decorModel;
    decorModel = nullptr;

    if (posterShadow)
    {
        posterShadow->deleteBuffers();
        delete posterShadow;
        posterShadow = nullptr;
    }

    if (posterPart)
    {
        posterPart->deleteBuffers();
        delete posterPart;
        posterPart = nullptr;
    }

    glDeleteTextures(static_cast<GLsizei>(posterTexture.size()), posterTexture.data());
    glDeleteTextures(static_cast<GLsizei>(zoneTexture.size()), zoneTexture.data());
    glDeleteTextures(static_cast<GLsizei>(tableTextures.size()), tableTextures.data());
    glDeleteTextures(static_cast<GLsizei>(roomTextures.size()), roomTextures.data());
}