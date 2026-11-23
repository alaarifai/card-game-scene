#include "Card.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static std::vector<vec3> makeCardVertices()
{
    return {
        vec3(-0.5f, 0.0f, 0.0f),
        vec3(0.5f, 0.0f, 0.0f),
        vec3(0.5f, 0.0f, -1.5f),
        vec3(-0.5f, 0.0f, -1.5f)};
}

static std::vector<vec2> makeCardTexCoords()
{
    return {
        vec2(0.0f, 0.0f),
        vec2(1.0f, 0.0f),
        vec2(1.0f, 1.0f),
        vec2(0.0f, 1.0f)};
}

Card::Card(int cardId, GLuint cardTexture, vec3 startPosition, float offset)
    : id(cardId),
      position(startPosition),
      angle(0.0f),
      stackOffset(offset),
      texture(cardTexture),
      shape(makeCardVertices(), makeCardTexCoords())
{
    shape.setTexture(texture);
}

void Card::draw(Shader &shader)
{
    mat4 model = mat4(1.0f);
    model = translate(model, position);
    model = rotate(model, radians(angle), vec3(1.0f, 0.0f, 0.0f));
    model = translate(model, vec3(0.0f, stackOffset, 0.0f));

    shape.transformation(model);
    shape.draw(shader);
}
void Card::stand()
{
    angle = 90.0f;
}

void Card::lay()
{
    angle = 0.0f;
}