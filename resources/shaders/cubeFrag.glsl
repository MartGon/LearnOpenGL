#version 400 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

struct Light
{
    vec3 pos;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform float intensity;
uniform float time;

uniform Light light;

uniform vec3 viewPos;

in vec3 normal;
in vec3 fragPos;
in vec2 textureCoords;
out vec4 fragColor;

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoords));

    vec3 lightDir = normalize(fragPos - light.pos);
    vec3 norm = normalize(normal);
    float diff = max(-dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoords));

    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(viewPos - fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoords));
    
    vec2 eTexCoords = textureCoords + vec2(0.0f, time * 0.1f);
    vec3 emission = vec3(texture(material.emission, eTexCoords)) * floor(vec3(1.0f) - vec3(texture(material.specular, textureCoords)));

    vec3 color = (ambient + diffuse + specular + emission * intensity);
    fragColor = vec4(color, 1.0f);
}