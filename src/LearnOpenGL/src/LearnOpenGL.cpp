#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;

bool isKeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

float Clamp(float value, float limit)
{
    return value > limit ? limit : value;
}

float Limit(float value, float downLimit, float upLimit)
{
    if(value > upLimit)
        return upLimit;
    else if(value < downLimit)
        return downLimit;

    return value;
}

// Camera
float lastX = WINDOW_WIDTH / 2;
float lastY = WINDOW_HEIGHT / 2;
float pitch = 0;
float yaw = glm::radians(270.0f);
bool firstMouse = true;
float fov = 45.0f;
const float rotationRate = 5.0f;
const float sensitivity = 0.0025f;

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{   
    auto xOffset = xpos - lastX;
    auto yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if(!firstMouse)
    {
        yaw += (xOffset * sensitivity);
        pitch += (yOffset * sensitivity);
        pitch = Limit(pitch, glm::radians(-90.0f), glm::radians(90.0f));
    }
    else
        firstMouse = false;
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    fov = Limit(fov, 1.0f, 90.0f);
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
            WINDOW_WIDTH = width;
            WINDOW_HEIGHT = height;
            glViewport(0, 0, width, height);
        }); 

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetScrollCallback(window, ScrollCallback);

        if(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            float vertices[] = {
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
                0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
            };
            unsigned int indices[] = 
            {
                0, 1, 3,
                1, 2, 3
            };

            glm::vec3 cubePos[] = {
                glm::vec3( 0.0f,  0.0f,  0.0f), 
                glm::vec3( 2.0f,  5.0f, -15.0f), 
                glm::vec3(-1.5f, -2.2f, -2.5f),  
                glm::vec3(-3.8f, -2.0f, -12.3f),  
                glm::vec3( 2.4f, -0.4f, -3.5f),  
                glm::vec3(-1.7f,  3.0f, -7.5f),  
                glm::vec3( 1.3f, -2.0f, -2.5f),  
                glm::vec3( 1.5f,  2.0f, -2.5f), 
                glm::vec3( 1.5f,  0.2f, -1.5f), 
                glm::vec3(-1.3f,  1.0f, -1.5f)  
            };
            int cubePosSize = sizeof(cubePos);
            std::cout << "cubePos size is " << cubePosSize << '\n';

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
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Camera system
            glm::vec3 cameraPos{0, 0, 3};
            glm::vec3 cameraFront{0, 0, -1.0f};
            glm::vec3 up{0, 1, 0};

            // Game loop
            float delta = 0;
            while(!glfwWindowShouldClose(window))
            {   
                float now = glfwGetTime();
                // Window Input
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, true);

                // Clear
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // View
                glm::vec3 direction;
                direction.x = cos(yaw) * cos(pitch);
                direction.y = sin(pitch);
                direction.z = sin(yaw) * cos(pitch);

                //std::cout << "Direction is now " << direction.x << ", " << direction.y << ", " << direction.z << "\n";
                cameraFront = glm::normalize(direction);

                float cameraSpeed = 2.5f * delta;
                if(isKeyPressed(window, GLFW_KEY_W))
                    cameraPos += cameraFront * cameraSpeed;
                else if(isKeyPressed(window, GLFW_KEY_S))
                    cameraPos -= cameraFront * cameraSpeed;

                if(isKeyPressed(window, GLFW_KEY_A))
                    cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, up));
                else if(isKeyPressed(window, GLFW_KEY_D))
                    cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, up));

                glm::vec3 target = cameraPos + cameraFront;
                auto view = glm::lookAt(cameraPos, target, glm::vec3{0, 1, 0});
                    
                shaderProg.setMatrix("view", glm::value_ptr(view));

                // Projection
                auto projection = glm::perspective(glm::radians(fov),  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.f);
                shaderProg.setMatrix("projection", glm::value_ptr(projection));

                // Rotation
                for(auto i = 0; i < 10; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, cubePos[i]);
                    float angle = glm::radians(20.0f * i);
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
                    shaderProg.setMatrix("model", glm::value_ptr(model));

                    // Draw
                    // Note: This triggers a segfault if the VerterAttribPointer of a in var is not defined
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }

                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);

                delta = glfwGetTime() - now;
            }
        }
    }

    // Close
    glfwTerminate();
}