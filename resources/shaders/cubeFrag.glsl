#version 420 core

uniform sampler2D texture_diffuse0;

in vec2 texCoords;

out vec4 FragColor;

void main()
{
    FragColor = texture(texture_diffuse0, texCoords);   
}  