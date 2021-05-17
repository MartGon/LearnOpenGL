#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool reverseNormals;

// Normal Mapping

// Light
struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Attenuation
struct Attenuation
{
    float constant;
    float linear;
    float quadratic;
};

// PointLight
struct PointLight
{
    vec3 pos;
    Attenuation attenuation;
    Light light;
};

#define POINT_LIGHTS_COUNT 4
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform vec3 viewPos;

out vec3 normal;
out vec3 fragPos;
out vec2 textureCoords;

out mat3 tbn;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

void main()
{
    mat4 transform = projection * view * model;

    gl_Position = transform * vec4(aPos, 1.0f);
    
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    normal = normalMatrix * (reverseNormals ? -aNormal : aNormal);
    fragPos = vec3(model * vec4(aPos, 1.0));
    textureCoords = aTextureCoords;
    tbn = mat3(normalMatrix * aTangent, normalMatrix * aBitangent, normal);

    tangentLightPos = tbn * pointLights[0].pos;
    tangentViewPos = tbn * viewPos;
    tangentFragPos = tbn * fragPos;
}