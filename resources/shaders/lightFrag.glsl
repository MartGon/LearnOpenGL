#version 400 core
out vec4 fragColor;

uniform vec3 color;

void main()
{
    fragColor = vec4(vec3(gl_FragCoord.z), 1.0f);
}