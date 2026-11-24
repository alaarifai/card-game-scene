#include "Room.h"
#include "tools/TextureLoader.h"
#include "tools/Box.h"

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

// Room data lives here (file-level), so main only calls buildRoom/drawRoom
static std::vector<GLuint> roomTextures;            // [0] = floor, [1] = wallpaper
static std::vector<PolygonWithTexture*> roomParts;  // floor, 4 walls, ceiling

// The table: one top slab + 4 legs (all built from Box), with its own textures
static std::vector<GLuint> tableTextures;           // [0] = green felt (top), [1] = dark wood (legs)
static std::vector<Box*> tableParts;                // top, then 4 legs

// Room limits (same numbers as the contract): x,z in [-15, 15], floor y = -3, ceiling y = 9
static const float ROOM = 15.0f;
static const float FLOOR_Y = -3.0f;
static const float CEIL_Y = 9.0f;

// Helper: builds one textured quad from 4 corners and 4 UVs, and gives it a texture
static void addQuad(const std::vector<glm::vec3>& corners, const std::vector<glm::vec2>& uvs, GLuint& texture)
{
    PolygonWithTexture* quad = new PolygonWithTexture(corners, uvs);
    quad->setTexture(texture);
    roomParts.push_back(quad);
}

void buildRoom()
{
    // 1. Load textures with GL_REPEAT so they can tile
    std::vector<std::string> paths = { "textures/floor.jpg", "textures/wall.jpg" };
    roomTextures = loadTextures(paths, GL_REPEAT, GL_LINEAR);
    if (roomTextures.size() != paths.size())
    {
        std::cerr << "Room: expected " << paths.size() << " textures, got " << roomTextures.size() << std::endl;
        return;
    }

    // 2. Floor: UV up to 10 so the wood repeats 10 x 10 times instead of stretching
    addQuad(
        {
            glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y,  ROOM),
            glm::vec3( ROOM, FLOOR_Y,  ROOM), glm::vec3( ROOM, FLOOR_Y, -ROOM)
        },
        { glm::vec2(0.0f, 10.0f), glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 0.0f), glm::vec2(10.0f, 10.0f) },
        roomTextures[0]);

    // 3. Walls: 30 wide x 12 tall. One wallpaper tile is about 6 x 6 units,
    //    so U goes 0 -> 5 along the width and V goes 0 -> 2 along the height (no stretching).
    std::vector<glm::vec2> wallUV =
        { glm::vec2(0.0f, 2.0f), glm::vec2(0.0f, 0.0f), glm::vec2(5.0f, 0.0f), glm::vec2(5.0f, 2.0f) };

    // corners order: top-left, bottom-left, bottom-right, top-right (as seen from INSIDE the room)
    addQuad( // back wall (z = -15)
        { glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y, -ROOM),
          glm::vec3( ROOM, FLOOR_Y, -ROOM), glm::vec3( ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad( // front wall (z = +15), behind the player
        { glm::vec3( ROOM, CEIL_Y,  ROOM), glm::vec3( ROOM, FLOOR_Y,  ROOM),
          glm::vec3(-ROOM, FLOOR_Y,  ROOM), glm::vec3(-ROOM, CEIL_Y,  ROOM) }, wallUV, roomTextures[1]);
    addQuad( // left wall (x = -15)
        { glm::vec3(-ROOM, CEIL_Y,  ROOM), glm::vec3(-ROOM, FLOOR_Y,  ROOM),
          glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad( // right wall (x = +15)
        { glm::vec3( ROOM, CEIL_Y, -ROOM), glm::vec3( ROOM, FLOOR_Y, -ROOM),
          glm::vec3( ROOM, FLOOR_Y,  ROOM), glm::vec3( ROOM, CEIL_Y,  ROOM) }, wallUV, roomTextures[1]);

    // 4. Ceiling at y = 9, same wallpaper, tiled 5 x 5.
    //    Corners are ordered counter-clockwise as seen from BELOW, so its front face points into the room.
    //    This lets drawRoom hide it when the top camera (y = 11) looks down from above.
    addQuad(
        {
            glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3( ROOM, CEIL_Y, -ROOM),
            glm::vec3( ROOM, CEIL_Y,  ROOM), glm::vec3(-ROOM, CEIL_Y,  ROOM)
        },
        { glm::vec2(0.0f, 5.0f), glm::vec2(5.0f, 5.0f), glm::vec2(5.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        roomTextures[1]);

    // 5. The table (contract: 12 x 8, top surface at y = 0).
    //    Top slab is 0.3 thick, so its center is at y = -0.15 and its size is (12, 0.3, 8).
    std::vector<std::string> tablePaths = { "textures/table_felt.jpg", "textures/table_wood.jpg" };
    tableTextures = loadTextures(tablePaths, GL_REPEAT, GL_LINEAR);
    if (tableTextures.size() != tablePaths.size())
    {
        std::cerr << "Room: table textures failed to load" << std::endl;
        return;
    }
    // Small helper so each table piece is one short line: (center, size, texture index)
    auto addBox = [](glm::vec3 center, glm::vec3 size, int tex)
    {
        tableParts.push_back(new Box(center, size, tableTextures[tex]));
    };
    const int FELT = 0, WOOD = 1;

    // Felt: the playing surface. Its top is exactly y = 0 (the contract), size 12 x 8.
    addBox(glm::vec3(0.0f, -0.15f, 0.0f), glm::vec3(12.0f, 0.3f, 8.0f), FELT);

    // Wooden frame around the felt (4 bars, 0.4 wide). Slightly higher (+0.06) so the felt looks sunk inside it.
    addBox(glm::vec3(0.0f, -0.12f,  4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);   // near the player
    addBox(glm::vec3(0.0f, -0.12f, -4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);   // far side
    addBox(glm::vec3( 6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);    // right
    addBox(glm::vec3(-6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);    // left

    // Wooden skirt (apron) under the top, between the legs
    addBox(glm::vec3(0.0f, -0.6f,  4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3(0.0f, -0.6f, -4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3( 6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);
    addBox(glm::vec3(-6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);

    // 4 legs from under the slab (y = -0.3) down to the floor (y = -3): height 2.7, center y = -1.65.
    // Each leg also gets a wider block at the top and a wider foot at the bottom, like the reference picture.
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
        {
            float x = sx * 6.0f, z = sz * 4.0f;
            addBox(glm::vec3(x, -1.65f, z),   glm::vec3(0.5f, 2.7f, 0.5f),   WOOD);  // leg
            addBox(glm::vec3(x, -0.85f, z),   glm::vec3(0.7f, 0.3f, 0.7f),   WOOD);  // top block
            addBox(glm::vec3(x, -2.85f, z),   glm::vec3(0.7f, 0.3f, 0.7f),   WOOD);  // foot
        }
}

void drawRoom(Shader& shader)
{
    shader.setInt("textureSample", 0);
    glActiveTexture(GL_TEXTURE0);
    // Floor and walls first (the ceiling is always the LAST part)
    for (size_t i = 0; i + 1 < roomParts.size(); i++)
        roomParts[i]->draw(shader);

    // The table (top + 4 legs), drawn before the ceiling
    for (Box* part : tableParts)
        part->draw(shader);

    // Ceiling: culled when seen from above, so the top camera can see the table through it
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
    glDeleteTextures(static_cast<GLsizei>(tableTextures.size()), tableTextures.data());
    glDeleteTextures(static_cast<GLsizei>(roomTextures.size()), roomTextures.data());
}