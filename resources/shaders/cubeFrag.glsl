#version 460 core
out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform mat4 view;

in vec3 normal;
in vec3 fragPos;

void main()
{   
    vec3 viewLightPos = vec3(view * vec4(lightPos, 1.0));
    float ambientStrength = 0.1f;
    vec3 ambient = lightColor * ambientStrength;

    vec3 lightDir = normalize(fragPos - viewLightPos);
    vec3 norm = normalize(normal);
    float lightMag = max(-dot(lightDir, normal), 0.0);
    vec3 diffuse = lightMag * lightColor;

    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(-fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specularStr = 0.5;
    vec3 specular = specularStr * spec * lightColor;

    vec3 color = (ambient + diffuse + specular) * objectColor;
    fragColor = vec4(color, 1.0f);
}