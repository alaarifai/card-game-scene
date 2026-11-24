#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Polygon.h"

// A reusable textured box: give it a center, a size (x, y, z) and a texture.
// Built from 6 textured faces of a cube with edge 1 that is scaled, then moved.
class Box
{
public:
    Box(glm::vec3 center, glm::vec3 size, GLuint texture, glm::vec3 color = glm::vec3(1.0f));

    void draw(Shader& shader);
    void destroy();

private:
    glm::mat4 model;
    std::vector<PolygonWithTexture> faces;
};