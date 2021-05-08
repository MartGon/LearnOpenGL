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
    float ratio = 1.00 / 1.52; // Air / Glass
    vec3 inputVec = fragPos - cameraPos;
    vec3 refracted = refract(inputVec, normalize(normal), ratio);
    fragColor = texture(Texture, refracted);
}