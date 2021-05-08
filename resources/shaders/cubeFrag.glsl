#version 400 core

// Uniforms
uniform sampler2D Texture;

// Inputs
in vec2 textureCoords;

// Output
out vec4 fragColor;

void main()
{
    fragColor = texture(Texture, textureCoords);
}