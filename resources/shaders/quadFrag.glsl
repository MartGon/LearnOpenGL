#version 420 core
out vec4 FragColor;

in vec2 textureCoords;

uniform sampler2D iTexture;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float near_plane = 1.0f;
    float far_plane = 7.5f;
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{            
    float near_plane = 1.0f;
    float far_plane = 7.5f; 
    float depthValue = texture(iTexture, textureCoords).r;
    //FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
    //FragColor = texture(iTexture, textureCoords);
}