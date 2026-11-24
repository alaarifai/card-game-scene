#include "Card.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// The card turns 90 degrees in 0.4 seconds and moves between zones in 0.7 seconds.
static const float ROTATION_SPEED = 90.0f / 0.4f;
static const float MOVE_DURATION = 0.7f;

// The card is a flat rectangle in the XZ plane. Its top face points toward +Y.
static std::vector<vec3> makeCardVertices()
{
    return {
        vec3(-0.5f, 0.0f, 0.0f),
        vec3(0.5f, 0.0f, 0.0f),
        vec3(0.5f, 0.0f, -1.5f),
        vec3(-0.5f, 0.0f, -1.5f)};
}

// Map the complete card image across the four rectangle corners.
static std::vector<vec2> makeCardTexCoords()
{
    return {
        vec2(0.0f, 0.0f),
        vec2(1.0f, 0.0f),
        vec2(1.0f, 1.0f),
        vec2(0.0f, 1.0f)};
}

// Store the card's ID, starting position, texture and rectangle geometry.
Card::Card(int cardId, GLuint cardTexture, vec3 startPos, float offset)
    : id(cardId),
      position(startPos),
      startPosition(startPos),
      targetPosition(startPos),
      progress(1.0f),
      moving(false),
      angle(0.0f),
      targetAngle(0.0f),
      stackOffset(offset),
      texture(cardTexture),
      shape(makeCardVertices(), makeCardTexCoords())
{
    shape.setTexture(texture);
}

// Begin a smooth movement from the current position to a new table position.
void Card::moveTo(vec3 target)
{
    startPosition = position;
    targetPosition = target;
    progress = 0.0f;
    moving = true;
    stackOffset = 0.0f;
}

// Update both the card's rotation and its movement using the elapsed frame time.
void Card::update(float deltaTime)
{
    float step = ROTATION_SPEED * deltaTime;

    if (angle < targetAngle)
        angle = min(angle + step, targetAngle);
    else if (angle > targetAngle)
        angle = max(angle - step, targetAngle);

    if (moving)
    {
        progress += deltaTime / MOVE_DURATION;

        if (progress >= 1.0f)
        {
            progress = 1.0f;
            moving = false;
        }

        // Smoothstep makes the movement start and stop gently instead of jumping.
        float t = progress * progress * (3.0f - 2.0f * progress);
        position = mix(startPosition, targetPosition, t);
        // Raise the card slightly in the middle to create a simple dealing arc.
        position.y += 0.8f * sin(3.14159265f * progress);
    }
}

// Build the model matrix, apply the current transform and draw the textured quad.
void Card::draw(Shader &shader)
{
    mat4 model = mat4(1.0f);
    model = translate(model, position);
    model = rotate(model, radians(angle), vec3(1.0f, 0.0f, 0.0f));
    model = translate(model, vec3(0.0f, stackOffset, 0.0f));

    shape.transformation(model);
    shape.draw(shader);
}

// Camera 2 shows the card vertically from the player's side.
void Card::stand()
{
    targetAngle = 90.0f;
}

// Camera 1 looks down at the table, so the card lies flat.
void Card::lay()
{
    targetAngle = 0.0f;
}
