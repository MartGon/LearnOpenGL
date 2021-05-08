#version 420 core

// Uniforms
uniform vec3 color;

// Output
out vec4 fragColor;

void main()
{
    fragColor = vec4(color, 1.0f);
}