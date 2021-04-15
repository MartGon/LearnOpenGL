#version 460 core
out vec4 fragColor;

in vec3 ourColor;
in vec2 textureCoords;

uniform sampler2D ourTexture;

void main()
{
    //fragColor = texture(ourTexture, textureCoords) * vec4(ourColor, 1.0f);
    fragColor = texture(ourTexture, textureCoords);
}