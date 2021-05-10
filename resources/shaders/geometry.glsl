#version 420 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in vec3 outColor[];
out vec3 color;

void main() {    

    color = outColor[0];
    
    gl_Position = gl_in[0].gl_Position + vec4(-0.2, -0.2, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.2, -0.2, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-0.2, 0.2, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.2, 0.2, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.4, 0.0, 0.0);
    color = vec3(1.0, 1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}  