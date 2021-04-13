#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>


struct Shader
{
    GLenum type;
    const char* source;
    unsigned int ref;
};

unsigned int CreateProgram(std::vector<Shader> shaders)
{
    unsigned int program =  glCreateProgram();
    for(auto shader : shaders)
    {
        shader.ref = glCreateShader(shader.type);
        glShaderSource(shader.ref, 1, &shader.source, 0);
        glCompileShader(shader.ref);

        int sucess = false;
        glGetShaderiv(shader.ref, GL_COMPILE_STATUS, &sucess);
        if(sucess)
        {
            glAttachShader(program, shader.ref);
        }
        else
        {
            char infoLog[512];
            glGetShaderInfoLog(shader.ref, 512, NULL, infoLog);
            std::cout << "Error while compiling shader " << infoLog << '\n';
            exit(-1);
        }
    }

    glLinkProgram(program);
    int status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status)
    {
        for(auto shader : shaders)
        {
            glDeleteShader(shader.ref);
        }
    }
    else
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Error while linking program " << infoLog << '\n';
        exit(-1);
    }

    return program;
}

unsigned int CreateVertexArray(float* vertices, unsigned int size)
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    return VAO;
}

int main()
{
    std::cout << "Hello world!\n";

    // GLFG setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(640, 480, "Window", NULL, NULL);
    if(window)
    {
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);
        });  

        if(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glViewport(0, 0, 640, 480);

            // Vertices
            float verticesTri1[] = {
                0.0f, 0.75f, 0.0f,  // up
                -0.5f, 0.15f, 0.0f, // down left
                0.5f, 0.15f, 0.0f, // down right
            };
            float verticesTri2[] = {
                0.0f, -0.75f, 0.0f,  // down
                -0.5f, -0.15f, 0.0f, // up left
                0.5f, -0.15f, 0.0f, // up right
            };

            const char* vertexShaderSrc = R"axy(
                #version 460 core
                layout (location = 0) in vec3 aPos;

                void main()
                {
                    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);
                }
            )axy";
            Shader vertexShader{GL_VERTEX_SHADER, vertexShaderSrc};

            const char* fragmentShaderSrc = R"(
                #version 460 core
                out vec4 fragColor;

                void main()
                {
                    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
                }
            )";
            Shader fragmentShader{GL_FRAGMENT_SHADER, fragmentShaderSrc};

            const char* fragmentShader2Src = R"(
                #version 460 core
                out vec4 fragColor;

                void main()
                {
                    fragColor = vec4(1.0f, 1.0f, 0.2f, 1.0f);
                }
            )";
            Shader fragmentShader2{GL_FRAGMENT_SHADER, fragmentShader2Src};

            unsigned int program = CreateProgram({vertexShader, fragmentShader});
            unsigned int program2 = CreateProgram({vertexShader, fragmentShader2});

            auto VAOtri1 = CreateVertexArray(verticesTri1, sizeof(verticesTri1));
            auto VAOtri2 = CreateVertexArray(verticesTri2, sizeof(verticesTri2));

            // Draw loop
            while(!glfwWindowShouldClose(window))
            {   
                // Window Input
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, true);

                // Clear
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Draw
                glUseProgram(program);
                glBindVertexArray(VAOtri1);
                glDrawArrays(GL_TRIANGLES, 0, 3);

                glUseProgram(program2);
                glBindVertexArray(VAOtri2);
                glDrawArrays(GL_TRIANGLES, 0, 3);

                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);  
            }
        }
    }

    // Close
    glfwTerminate();
}