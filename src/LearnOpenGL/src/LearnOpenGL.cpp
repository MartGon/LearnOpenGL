#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>

#include <Shader.h>

unsigned int GenTexture(unsigned char* data, int w, int h, int textureUnit = GL_TEXTURE0, int format = GL_RGB)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
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
               // positions          // colors           // texture coords (note that we changed them to 'zoom in' on our texture image)
                0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.f, 1.f, // top right
                0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.f, 0.0f, // bottom right
                -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
                -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
            };
            unsigned int indices[] = 
            {
                0, 1, 3,
                1, 2, 3
            };

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path fragPath = shaderFolder / "fragment.glsl";
            LearnOpenGL::Shader shaderProg{vertexPath.c_str(), fragPath.c_str()};
            shaderProg.use();

            // Texture loading
            std::filesystem::path textureFolder{TEXTURES_DIR};
            std::filesystem::path texturePath = textureFolder / "container.jpg";
            std::filesystem::path facePath = textureFolder / "awesomeface.png";
            stbi_set_flip_vertically_on_load(true);  

            int w, h;
            int channels;
            unsigned char* textureData = stbi_load(texturePath.c_str(), &w, &h, &channels, 0);

            // OpenGL Texture
            auto texture = GenTexture(textureData, w, h, GL_TEXTURE0);
            stbi_image_free(textureData);

            textureData = stbi_load(facePath.c_str(), &w, &h, &channels, 0);
            auto texture2 = GenTexture(textureData, w, h, GL_TEXTURE1, GL_RGBA);

            shaderProg.setInt("ourTexture1", 0);
            shaderProg.setInt("ourTexture2", 1);
            float mixRatio = 0.3f;
            const float MOD_RATE =  0.05f;

            // VAOs
            unsigned int VAO;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            
            unsigned int VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            unsigned int EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Vertex Attributes
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // Draw loop
            while(!glfwWindowShouldClose(window))
            {   
                // Window Input
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, true);

                // Clear
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Mod mixRatio
                if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && mixRatio < (1 - MOD_RATE))
                    mixRatio += MOD_RATE;
                else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && mixRatio > MOD_RATE)
                    mixRatio -= MOD_RATE;
                shaderProg.setFloat("mixRatio", mixRatio);

                // Draw
                // Note: This triggers a segfault if the VerterAttribPointer of a in var is not defined
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);  
            }
        }
    }

    // Close
    glfwTerminate();
}