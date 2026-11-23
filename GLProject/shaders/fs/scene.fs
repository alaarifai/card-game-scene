#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSample;
uniform vec3 objectColor;

void main()
{
    vec4 texColor = texture(textureSample, TexCoord);

    if (texColor.a < 0.1)

        discard;


    FragColor = texColor * vec4(objectColor, 1.0);
}