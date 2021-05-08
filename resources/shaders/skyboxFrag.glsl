#version 400 core

// Uniforms
uniform samplerCube Texture;

// Inputs
in vec3 fragPos;

// Output
out vec4 fragColor;

void main()
{
    fragColor = texture(Texture, fragPos);
}