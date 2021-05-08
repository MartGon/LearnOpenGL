#version 400 core

// Uniforms
uniform vec3 cameraPos;
uniform samplerCube Texture;

// Inputs
in vec3 normal;
in vec2 textureCoords;
in vec3 fragPos;

// Output
out vec4 fragColor;

void main()
{
    vec3 inputVec = fragPos - cameraPos;
    vec3 reflected = reflect(inputVec, normalize(normal));
    fragColor = texture(Texture, reflected);
}