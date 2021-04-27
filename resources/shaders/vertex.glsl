#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 fragPos;

void main()
{
    mat4 viewTransform = view * model;
    mat4 transform = projection * viewTransform;
    gl_Position = transform * vec4(aPos, 1.0f);

    mat3 normalMatrix = mat3(transpose(inverse(viewTransform)));
    normal = normalMatrix * aNormal;
    fragPos = vec3(viewTransform * vec4(aPos, 1.0));
}