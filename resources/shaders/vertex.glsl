#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTextureCoords;

uniform bool skybox;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 textureCoords;
out vec3 fragPos;

void main()
{
    mat4 transform = projection * view * model;

    gl_Position = transform * vec4(aPos, 1.0f);
    if(skybox)
    {
        gl_Position = vec4(gl_Position.xyww);
    }
    textureCoords = aTextureCoords;
    fragPos = aPos;
}