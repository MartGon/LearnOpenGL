#version 400 core

// Light
struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcAmbient(Light light);
vec3 CalcDiffuse(Light light, vec3 lightDir, vec3 normal);
vec3 CalcSpecular(Light light, vec3 lightDir, vec3 normal);
Light CalcColor(Light light, vec3 lightDir, vec3 normal);

// Attenuation
struct Attenuation
{
    float constant;
    float linear;
    float quadratic;
};

float CalcAttenuation(Attenuation attenuation, vec3 lightRay);

// DirLight
struct DirLight
{
    vec3 dir;
    Light light;
};

vec3 CalcDirLight(DirLight dirLight, vec3 normal);

// PointLight
struct PointLight
{
    vec3 pos;
    Attenuation attenuation;
    Light light;
};

vec3 CalcPointLight(PointLight pointLight, vec3 normal);

// SpotLight
struct SpotLight
{
    vec3 pos;
    vec3 dir;

    float iCutOff;
    float oCutOff;

    Attenuation attenuation;
    Light light;
};

float CalcIntensity(SpotLight spotLight);
vec3 CalcSpotLight(SpotLight spotLight, vec3 normal);

// Lights
uniform DirLight dirLight;
#define POINT_LIGHTS_COUNT 4
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform SpotLight spotLight;

// Material
struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};
uniform Material material;

// Flags
uniform bool sunOn;
uniform bool flashlightOn;
uniform bool lightsOn[POINT_LIGHTS_COUNT];

// View angle
uniform vec3 viewPos;

// Inputs
in vec3 normal;
in vec3 fragPos;
in vec2 textureCoords;

// Output
out vec4 fragColor;

void main()
{
    vec3 color = vec3(0.0);
    
    if(sunOn)
        color += CalcDirLight(dirLight, normal);

    for(int i = 0; i < 4; i++)
    {
        if(lightsOn[i])
            color += CalcPointLight(pointLights[i], normal);
    }
    

    if(flashlightOn)
        color += CalcSpotLight(spotLight, normal);

    vec4 textureColor = texture(material.texture_diffuse1, textureCoords);
    if(textureColor.a < 0.1)
        discard;

    fragColor = textureColor;
}

vec3 CalcDirLight(DirLight dirLight, vec3 normal)
{
    Light color = CalcColor(dirLight.light, dirLight.dir, normal);
    return color.ambient + color.diffuse + color.specular;
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal)
{
    vec3 lightRay = fragPos - pointLight.pos;
    float attenuation = CalcAttenuation(pointLight.attenuation, lightRay);
    Light color = CalcColor(pointLight.light, lightRay, normal);
    return (color.ambient + color.diffuse + color.specular)* attenuation;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 normal)
{
    vec3 lightRay = fragPos - spotLight.pos;
    float attenuation = CalcAttenuation(spotLight.attenuation, lightRay);
    float intensity = CalcIntensity(spotLight);
    Light color = CalcColor(spotLight.light, lightRay, normal);

    return (color.ambient + (color.diffuse + color.specular) * intensity) * attenuation;
}

float CalcIntensity(SpotLight spotLight)
{
    vec3 lightRay = fragPos - spotLight.pos;
    vec3 lightDir = normalize(lightRay);
    float theta = dot(lightDir, normalize(spotLight.dir));
    float epsilon = spotLight.iCutOff - spotLight.oCutOff;
    float intensity = clamp((theta - spotLight.oCutOff) / epsilon, 0.0, 1.0);

    return intensity;
}

float CalcAttenuation(Attenuation attenuation, vec3 lightRay)
{
    float dist = length(lightRay);
    float att = 1.0 / (attenuation.constant + attenuation.linear * dist + attenuation.quadratic * pow(dist, 2));
    return att;
}

Light CalcColor(Light light, vec3 lightDir, vec3 normal)
{   
    lightDir = normalize(lightDir);
    normal = normalize(normal);

    Light color;
    color.ambient = CalcAmbient(light);
    color.diffuse = CalcDiffuse(light, lightDir, normal);
    color.specular = CalcSpecular(light, lightDir, normal);
    return color;
}

vec3 CalcAmbient(Light light)
{
    return light.ambient * vec3(texture(material.texture_diffuse1, textureCoords));
}

vec3 CalcDiffuse(Light light, vec3 lightDir, vec3 normal)
{
    float diff = max(dot(-lightDir, normal), 0.0);
    return light.diffuse * diff * texture(material.texture_diffuse1, textureCoords).rgb;
}

vec3 CalcSpecular(Light light, vec3 lightDir, vec3 normal)
{
    vec3 reflected = reflect(lightDir, normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    float spec = pow(max(dot(reflected, viewDir), 0.0), material.shininess);

    return light.specular * spec * texture(material.texture_specular1, textureCoords).rgb;
}