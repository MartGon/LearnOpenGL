#version 460 core
out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = lightColor * ambientStrength;
    vec3 color = ambient * objectColor;
    fragColor = vec4(color, 1.0f);
}