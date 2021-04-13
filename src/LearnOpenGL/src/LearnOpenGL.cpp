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
            float vertices[] = {
                0, 0.75f, 0,
                -0.5f, 0.10, 0,
                0.5f, 0.10, 0,

                0, -0.75f, 0,
                -0.5f, -0.10f, 0,
                0.5f, -0.10f, 0
            };
            unsigned int indices[] = 
            {
                0, 1, 3,
                1, 2, 3
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

            unsigned int program = CreateProgram({vertexShader, fragmentShader});
            glUseProgram(program);

            unsigned int VAO;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            
            unsigned int VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            /*
            unsigned int EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            */

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(0);

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
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);  
            }
        }
    }

    // Close
    glfwTerminate();
}