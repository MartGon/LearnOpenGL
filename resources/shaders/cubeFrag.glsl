#version 400 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light
{
    vec3 pos;
    vec3 dir;
    float iCutOff;
    float oCutOff;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

in vec3 normal;
in vec3 fragPos;
in vec2 textureCoords;
out vec4 fragColor;

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoords));

    vec3 lightVec = fragPos - light.pos;
    vec3 lightDir = normalize(lightVec);
    float theta = dot(lightDir, light.dir);
    float epsilon = light.iCutOff - light.oCutOff;
    float intensity = clamp((theta - light.oCutOff) / epsilon, 0.0, 1.0);

    vec3 color;
    float dist = length(lightVec);
    vec3 norm = normalize(normal);
    float diff = max(dot(-lightDir, normal), 0.0);
    float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * pow(dist, 2));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoords));

    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(viewPos - fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoords));

    color = (ambient + (diffuse + specular) * intensity) * attenuation;
    

    fragColor = vec4(color, 1.0f);
}