#version 460 core
out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

in vec3 normal;
in vec3 fragPos;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = lightColor * ambientStrength;

    vec3 lightDir = normalize(fragPos - lightPos);
    vec3 norm = normalize(normal);
    float lightMag = max(-dot(lightDir, normal), 0.0);
    vec3 diffuse = lightMag * lightColor;

    vec3 color = (ambient + diffuse) * objectColor;

    fragColor = vec4(color, 1.0f);
}