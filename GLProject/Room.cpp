#include "Room.h"
#include "tools/TextureLoader.h"

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

// Room data lives here (file-level), so main only calls buildRoom/drawRoom
static std::vector<GLuint> roomTextures;            // [0] = floor, [1] = wallpaper
static std::vector<PolygonWithTexture*> roomParts;  // floor, 4 walls, ceiling

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

    // 4. Ceiling at y = 9, same wallpaper, tiled 5 x 5
    addQuad(
        {
            glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3(-ROOM, CEIL_Y,  ROOM),
            glm::vec3( ROOM, CEIL_Y,  ROOM), glm::vec3( ROOM, CEIL_Y, -ROOM)
        },
        { glm::vec2(0.0f, 5.0f), glm::vec2(0.0f, 0.0f), glm::vec2(5.0f, 0.0f), glm::vec2(5.0f, 5.0f) },
        roomTextures[1]);
}

void drawRoom(Shader& shader)
{
    shader.setInt("textureSample", 0);
    glActiveTexture(GL_TEXTURE0);
    for (PolygonWithTexture* part : roomParts)
        part->draw(shader);
}

void destroyRoom()
{
    for (PolygonWithTexture* part : roomParts)
    {
        part->deleteBuffers();
        delete part;
    }
    roomParts.clear();
    glDeleteTextures(static_cast<GLsizei>(roomTextures.size()), roomTextures.data());
}