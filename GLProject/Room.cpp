#include "Room.h"
#include "tools/TextureLoader.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

// Room data lives here (file-level), so main only calls buildRoom/drawRoom
static std::vector<GLuint> roomTextures;
static PolygonWithTexture* floorPoly = nullptr;

void buildRoom()
{
    // 1. Load the floor texture with GL_REPEAT so it can tile
    std::vector<std::string> paths = { "textures/floor.jpg" };
    roomTextures = loadTextures(paths, GL_REPEAT, GL_LINEAR);

    // 2. Floor: x,z from -15 to +15 at height y = -3 (counter-clockwise seen from above)
    std::vector<glm::vec3> vertices =
    {
        glm::vec3(-15.0f, -3.0f, -15.0f), // back-left
        glm::vec3(-15.0f, -3.0f,  15.0f), // front-left
        glm::vec3( 15.0f, -3.0f,  15.0f), // front-right
        glm::vec3( 15.0f, -3.0f, -15.0f)  // back-right
    };

    // 3. UV goes up to 10 so the wood repeats 10 x 10 times instead of stretching
    std::vector<glm::vec2> texcoords =
    {
        glm::vec2( 0.0f, 10.0f),
        glm::vec2( 0.0f,  0.0f),
        glm::vec2(10.0f,  0.0f),
        glm::vec2(10.0f, 10.0f)
    };

    floorPoly = new PolygonWithTexture(vertices, texcoords);
    floorPoly->setTexture(roomTextures[0]);
}

void drawRoom(Shader& shader)
{
    floorPoly->draw(shader);
}

void destroyRoom()
{
    floorPoly->deleteBuffers();
    delete floorPoly;
    floorPoly = nullptr;
    glDeleteTextures(static_cast<GLsizei>(roomTextures.size()), roomTextures.data());
}