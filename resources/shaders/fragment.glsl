#version 460 core
out vec4 fragColor;

in vec2 textureCoords;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    fragColor = mix(texture(ourTexture1, textureCoords), texture(ourTexture2, textureCoords), 0.3);
}