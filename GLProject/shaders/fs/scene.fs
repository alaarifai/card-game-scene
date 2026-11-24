#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D textureSample;
uniform vec3 objectColor;
uniform float emissive;   // 1.0 = glowing surface (lantern glass): skip the lighting, keep the colour

// ---- Lights (kept in the shader on purpose: only Room.cpp knows where the lanterns hang) ----
const int NL = 5;
const vec3 LP[5] = vec3[5](
    vec3(-4.0, 3.4, 0.0),      // warm lantern, left of the table
    vec3( 4.0, 3.4, 0.0),      // warm lantern, right of the table
    vec3( 0.0, 8.0, 13.0),     // cool fill from the player's side, so the shadows stay readable
    vec3(-5.6, 2.7, -13.1),    // wall sconce, left of the poster
    vec3( 5.6, 2.7, -13.1));   // wall sconce, right of the poster
const vec3 LC[5] = vec3[5](
    vec3(1.00, 0.72, 0.40),
    vec3(1.00, 0.72, 0.40),
    vec3(0.30, 0.36, 0.52),
    vec3(1.00, 0.80, 0.50),
    vec3(1.00, 0.80, 0.50));
const float LR[5] = float[5](15.0, 15.0, 45.0, 12.0, 12.0);   // reach of each light
const float INTENSITY = 0.85;
const float AMBIENT = 0.42;                       // light that reaches even the far corners
const vec3 SKY = vec3(0.10, 0.11, 0.14);          // a little extra on surfaces that face up

void main()
{
    vec4 texColor = texture(textureSample, TexCoord);

    if (texColor.a < 0.1)
        discard;

    // Real normal for the 3D model; flat normal rebuilt from the geometry for polygons and boxes.
    vec3 N = normalize(Normal);
    if (dot(Normal, Normal) < 1e-6)
    {
        N = normalize(cross(dFdx(WorldPos), dFdy(WorldPos)));
        if (!gl_FrontFacing)
            N = -N;
    }

    vec3 light = vec3(AMBIENT) + SKY * max(N.y, 0.0);
    for (int i = 0; i < NL; i++)
    {
        vec3 toLight = LP[i] - WorldPos;
        float dist = length(toLight);
        float att = clamp(1.0 - dist / LR[i], 0.0, 1.0);   // linear falloff, squared = softer edge
        att *= att;
        float ndl = max(dot(N, toLight / max(dist, 0.0001)), 0.0);
        light += LC[i] * (ndl * att * INTENSITY);
    }

    vec3 base = texColor.rgb * objectColor;
    vec3 color = mix(base * light, base * 1.25, clamp(emissive, 0.0, 1.0));

    FragColor = vec4(min(color, vec3(1.0)), 1.0);
}

