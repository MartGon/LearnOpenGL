#version 460 core
out vec4 fragColor;

in vec3 ourColor;
in vec2 textureCoords;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;
uniform float mixRatio;

void main()
{
    fragColor = mix(texture(ourTexture1, textureCoords), texture(ourTexture2, textureCoords), mixRatio);
}