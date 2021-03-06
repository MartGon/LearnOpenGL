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

// Shadow
float CalcShadow(vec3 lightPos);

// Lights
uniform DirLight dirLight;
#define POINT_LIGHTS_COUNT 4
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform SpotLight spotLight;

// Material
struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D depth;
    float shininess;
};
uniform Material material;

// Parallax

vec2 CalculateParallaxCoords(vec2 texCoords, vec3 viewDir);
vec2 GetParallaxCoords();

// Flags
uniform bool blinn;
uniform bool sunOn;
uniform bool flashlightOn;
uniform bool lightsOn[POINT_LIGHTS_COUNT];

// View angle
uniform vec3 viewPos;

// Inputs
in vec3 normal;
in vec3 fragPos;
in vec2 textureCoords;
in vec4 fragPosLightSpace;

// Normal mapping
in mat3 tbn;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

// Output
out vec4 fragColor;

void main()
{
    vec3 color = vec3(0.0);
    vec2 texCoords = GetParallaxCoords();
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    vec3 sampledNormal = normalize(texture(material.normal, texCoords).rgb * 2.0 - 1.0);

    if(sunOn)
        color += CalcDirLight(dirLight, sampledNormal);

    for(int i = 0; i < 4; i++)
    {
        if(lightsOn[i])
            color += CalcPointLight(pointLights[i], sampledNormal);
    }
    
    if(flashlightOn)
        color += CalcSpotLight(spotLight, sampledNormal);

    float gamma = 2.2;
    color = pow(color, vec3(1/gamma));
    fragColor = vec4(color, 1.0f);
    //fragColor = texture(material.depth, textureCoords);
}

vec2 GetParallaxCoords()
{
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    return CalculateParallaxCoords(textureCoords, viewDir);
}

vec2 CalculateParallaxCoords(vec2 texCoords, vec3 viewDir)
{
    const float minLayers = 8.0f;
    const float maxLayers = 32.0f;
    const float height_scale = 0.1f;
    float numLayers = max(minLayers, round((1 - dot(viewDir, vec3(0.0f, 0.0f, 1.0f))) * maxLayers));
    float stepSize = 1 / numLayers;
    
    int layer = 0;
    vec2 currentTexCoords = texCoords;
    for(layer = 0; layer < numLayers; layer++)
    {
        float layerDepth = stepSize * layer;
        float offsetMagnitude = (1 + layerDepth) * height_scale;
        vec2 texOffset = viewDir.xy * offsetMagnitude;
        currentTexCoords = texCoords - texOffset;
        float currentDepthValue = texture(material.depth, currentTexCoords).r;

        if(layerDepth > currentDepthValue)
        {
            break;
        }
    }

    float layerDepth = stepSize * layer;
    float prevLayerDepth = stepSize * max(layer - 1, 0);

    vec2 prevTexCoords = texCoords - viewDir.xy  * (1 + prevLayerDepth) * height_scale;

    float depth = texture(material.depth, currentTexCoords).r;
    float prevDepth = texture(material.depth, prevTexCoords).r;

    float diff = layerDepth - depth;
    float prevDiff = prevDepth - prevLayerDepth;
    float weight = diff / (diff + prevDiff);
    // We are giving the most weight based on the least difference, that is distance to the real point
    // That's why the closest is diff to (diff + prevDiff), the closest is weight to 1 
    // and thus the less it's corresponding coords contribute to the interpolation.
    currentTexCoords = prevTexCoords * (weight) + currentTexCoords * (1 - weight);

    return currentTexCoords;
}

vec3 CalcDirLight(DirLight dirLight, vec3 normal)
{
    Light color = CalcColor(dirLight.light, dirLight.dir, normal);
    return (color.ambient + (color.diffuse + color.specular));
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal)
{
    vec3 lightRay = fragPos - pointLight.pos;
    float attenuation = CalcAttenuation(pointLight.attenuation, lightRay);
    Light color = CalcColor(pointLight.light, lightRay, normal);

    return (color.ambient + (color.diffuse + color.specular))* attenuation;
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
    return light.ambient * vec3(texture(material.diffuse, GetParallaxCoords()));
}

vec3 CalcDiffuse(Light light, vec3 lightDir, vec3 normal)
{
    lightDir = normalize(tangentFragPos - tangentLightPos);
    float diff = max(dot(-lightDir, normal), 0.0);
    return light.diffuse * diff * texture(material.diffuse, GetParallaxCoords()).rgb;
}

vec3 CalcSpecular(Light light, vec3 lightDir, vec3 normal)
{   
    lightDir = normalize(tangentFragPos - tangentLightPos);
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    float spec = 0;
    if(blinn)
    {
        vec3 halfway = normalize(viewDir - lightDir);
        spec = pow(max(dot(normal, halfway), 0.0), material.shininess * 2);
    }
    else
    {
        vec3 reflected = reflect(lightDir, normal);
        spec = pow(max(dot(reflected, viewDir), 0.0), material.shininess);
    }

    return light.specular * spec * texture(material.specular, GetParallaxCoords()).rgb;
}