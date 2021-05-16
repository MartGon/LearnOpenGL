#version 420 core

uniform vec3 lightPos;
uniform float far_plane;

in vec4 fragPos;

void main()
{             
    float lightDistance = length(fragPos.xyz - lightPos);
    gl_FragDepth = lightDistance / far_plane;
}  