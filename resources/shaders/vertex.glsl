#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool border;

out vec3 normal;
out vec3 fragPos;
out vec2 textureCoords;

void main()
{
    mat4 transform = projection * view * model;

    gl_Position = transform * vec4(aPos, 1.0f);
    textureCoords = aTextureCoords;
    
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    normal = normalMatrix * aNormal;
    fragPos = vec3(model * vec4(aPos, 1.0));
    if(border)
        gl_Position.z = 0.0;
}