#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    WorldPos = world.xyz;

    // The 3D model (loaded with Assimp) is the only thing that has a real normal attribute.
    // Our polygons and boxes don't, so there it stays zero and the fragment shader rebuilds
    // a flat normal from the screen-space derivatives of WorldPos.
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoord = aTexCoord;
    gl_Position = projection * view * world;
}
