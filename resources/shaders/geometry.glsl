#version 420 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float time;

in vec2 gTexCoords[];
out vec2 texCoords;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
} 

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

void main() {    

    vec3 gNormal = GetNormal();
    
    gl_Position = explode(gl_in[0].gl_Position, gNormal);
    texCoords = gTexCoords[0];
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, gNormal);
    texCoords = gTexCoords[1];
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, gNormal);
    texCoords = gTexCoords[2];
    EmitVertex();

    EndPrimitive();
}  