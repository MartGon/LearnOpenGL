#version 400 core

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
uniform Light light;
uniform vec3 lightColor;

uniform vec3 viewPos;

in vec3 normal;
in vec3 fragPos;
out vec4 fragColor;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 lightDir = normalize(fragPos - light.pos);
    vec3 norm = normalize(normal);
    float diff = max(-dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * material.diffuse * diff;

    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(viewPos - fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * material.specular * spec;

    vec3 color = (ambient + diffuse + specular);
    fragColor = vec4(color, 1.0f);
}