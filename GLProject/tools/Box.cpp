#include "Box.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;



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
    
    model = translate(mat4(1.0f), center) * scale(mat4(1.0f), size);

    const float h = 0.5f;
    faces.push_back(makeFace(vec3(-h, -h,  h), vec3( h, -h,  h), vec3( h,  h,  h), vec3(-h,  h,  h), texture, color)); 
    faces.push_back(makeFace(vec3( h, -h, -h), vec3(-h, -h, -h), vec3(-h,  h, -h), vec3( h,  h, -h), texture, color)); 
    faces.push_back(makeFace(vec3( h, -h,  h), vec3( h, -h, -h), vec3( h,  h, -h), vec3( h,  h,  h), texture, color)); 
    faces.push_back(makeFace(vec3(-h, -h, -h), vec3(-h, -h,  h), vec3(-h,  h,  h), vec3(-h,  h, -h), texture, color)); 
    faces.push_back(makeFace(vec3(-h,  h,  h), vec3( h,  h,  h), vec3( h,  h, -h), vec3(-h,  h, -h), texture, color)); 
    faces.push_back(makeFace(vec3(-h, -h, -h), vec3( h, -h, -h), vec3( h, -h,  h), vec3(-h, -h,  h), texture, color)); 
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