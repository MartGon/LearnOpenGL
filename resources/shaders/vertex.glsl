#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 normal;
out vec3 aFragColor;

void main()
{
    mat4 transform = projection * view * model;
    gl_Position = transform * vec4(aPos, 1.0f);

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    normal = normalMatrix * aNormal;
    
    float ambientStrength = 0.1f;
    vec3 ambient = lightColor * ambientStrength;

    vec3 fragPos = aPos;
    vec3 lightDir = normalize(fragPos - lightPos);
    vec3 norm = normalize(normal);
    float lightMag = max(-dot(lightDir, normal), 0.0);
    vec3 diffuse = lightMag * lightColor;

    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(viewPos - fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specularStr = 1;
    vec3 specular = specularStr * spec * lightColor;

    vec3 color = (ambient + diffuse + specular) * objectColor;
    aFragColor = color;
}