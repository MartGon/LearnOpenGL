#version 420 core
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec2 offset;

out vec3 color;

void main()
{
    gl_Position = vec4(vPos + offset, 0.0, 1.0);
    color = vColor;
}
