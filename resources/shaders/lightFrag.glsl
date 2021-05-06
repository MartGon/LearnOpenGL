#version 400 core
out vec4 fragColor;

uniform bool border;
uniform vec3 color;

out float gl_FragDepth;

void main()
{
    fragColor = vec4(color, 1.0f);
    gl_FragDepth = border ? 0 : gl_FragCoord.z;
}