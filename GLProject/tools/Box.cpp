#include "Box.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// Builds one face of the unit cube (edge 1, centered at the origin).
// Corners are given counter-clockwise as seen from OUTSIDE the cube.
static PolygonWithTexture makeFace(vec3 bl, vec3 br, vec3 tr, vec3 tl, GLuint texture, vec3 color)
{
    std::vector<vec3> corners = { bl, br, tr, tl };
    std::vector<vec2> uvs = { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f) };
    PolygonWithTexture face(corners, uvs, color);
    face.setTexture(texture);
    return face;
}

Box::Box(vec3 center, vec3 size, GLuint texture, vec3 color)
{
    // Scale first (size), then translate (center): T * S, so the scale does not move the box away
    model = translate(mat4(1.0f), center) * scale(mat4(1.0f), size);

    const float h = 0.5f;
    faces.push_back(makeFace(vec3(-h, -h,  h), vec3( h, -h,  h), vec3( h,  h,  h), vec3(-h,  h,  h), texture, color)); // front  (+z)
    faces.push_back(makeFace(vec3( h, -h, -h), vec3(-h, -h, -h), vec3(-h,  h, -h), vec3( h,  h, -h), texture, color)); // back   (-z)
    faces.push_back(makeFace(vec3( h, -h,  h), vec3( h, -h, -h), vec3( h,  h, -h), vec3( h,  h,  h), texture, color)); // right  (+x)
    faces.push_back(makeFace(vec3(-h, -h, -h), vec3(-h, -h,  h), vec3(-h,  h,  h), vec3(-h,  h, -h), texture, color)); // left   (-x)
    faces.push_back(makeFace(vec3(-h,  h,  h), vec3( h,  h,  h), vec3( h,  h, -h), vec3(-h,  h, -h), texture, color)); // top    (+y)
    faces.push_back(makeFace(vec3(-h, -h, -h), vec3( h, -h, -h), vec3( h, -h,  h), vec3(-h, -h,  h), texture, color)); // bottom (-y)
}

void Box::draw(Shader& shader)
{
    glActiveTexture(GL_TEXTURE0);
    for (PolygonWithTexture& face : faces)
    {
        face.transformation(model);
        face.draw(shader);
    }
}

void Box::destroy()
{
    for (PolygonWithTexture& face : faces)
        face.deleteBuffers();
    faces.clear();
}