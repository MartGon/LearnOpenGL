#version 460 core
out vec4 fragColor;

in vec3 aFragColor;

void main()
{
    fragColor = vec4(aFragColor, 1.0f);
}