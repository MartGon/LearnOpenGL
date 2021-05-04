#version 400 core
out vec4 fragColor;

uniform vec3 color;
float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    float z = LinearizeDepth(gl_FragCoord.z) / far;
    fragColor = vec4(vec3(z), 1.0f);
}