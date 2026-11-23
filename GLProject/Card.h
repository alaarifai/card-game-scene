#pragma once

#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>
#include "tools/Polygon.h"

class Card
{
public:
    Card(int cardId, GLuint cardTexture, glm::vec3 startPosition, float offset);

    void update(float deltaTime);
    void draw(Shader &shader);
    void stand();
    void lay();

    int id;
    glm::vec3 position;
    float angle;
    float targetAngle;
    float stackOffset;

private:
    GLuint texture;
    PolygonWithTexture shape;
};