#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos;

uniform sampler2D textureSample;
uniform vec3 objectColor;
uniform bool glow;   


const vec3 lightPos[4] = vec3[4](
    vec3(-4.0, 3.4, 0.0),
    vec3( 4.0, 3.4, 0.0),
    vec3(-5.6, 2.7, -13.1),
    vec3( 5.6, 2.7, -13.1));

const vec3 lightColor = vec3(1.0, 0.75, 0.45);   
const float ambient = 0.4;                       
const float intensity = 0.55;

void main()
{
    vec4 texColor = texture(textureSample, TexCoord);

    
    if (texColor.a < 0.1)
        discard;

    vec3 color = texColor.rgb * objectColor;

    if (glow)
    {
        FragColor = vec4(min(color * 1.25, vec3(1.0)), 1.0);
        return;
    }

    
    vec3 light = vec3(ambient);
    for (int i = 0; i < 4; i++)
    {
        float distance = length(lightPos[i] - WorldPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        light += lightColor * attenuation * intensity;
    }

    FragColor = vec4(min(color * light, vec3(1.0)), 1.0);
}
