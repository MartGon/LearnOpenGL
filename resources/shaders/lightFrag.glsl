#version 400 core
out vec4 fragColor;

uniform vec3 color;
float near = 0.1; 
float far  = 100.0; 

void main()
{
    fragColor = vec4(vec3(color), 1.0f);
}