#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include <random>

#include <Shader.h>
#include <Camera.h>
#include <Mesh.h>
#include <Model.h>

std::filesystem::path GetRelativePath(std::filesystem::path dir, std::filesystem::path filename)
{
    return std::filesystem::path{dir/filename};
}

unsigned int GenTexture(std::filesystem::path path, int textureUnit = GL_TEXTURE0, int format = GL_RGB, int textureType = GL_TEXTURE_2D)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    auto data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
    if(data)
    {
        glTexImage2D(textureType, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Error:" << stbi_failure_reason() <<" while loading texture at " << path.string().c_str() << '\n';
        return -1;
    }
    stbi_image_free(data);

    return texture;
}

unsigned int GenCubeMap(std::vector<std::filesystem::path> faces)
{
    unsigned int cubeMap;
    glGenTextures(1, &cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

    for(unsigned int i = 0; i < faces.size(); i++)
    {
        auto path = faces[i];
        int width, height, nrChannels;
        auto data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
        if(data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubeMap;
}

bool isKeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool isKeyReleased(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_RELEASE;
}

// Camera
Camera camera;
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;
float lastX = WINDOW_WIDTH / 2;
float lastY = WINDOW_HEIGHT / 2;

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{   
    auto xOffset = xpos - lastX;
    auto yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xOffset, yOffset, true);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

int main()
{
    std::cout << "Hello world!\n";

    // GLFG setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Window", NULL, NULL);
    glfwSetWindowPos(window, 1920 * 2 * 3/4 - WINDOW_WIDTH / 2 , 1080 / 2 - WINDOW_HEIGHT / 2);
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
            glEnable(GL_MULTISAMPLE);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            // Cube vertices
            float cubeVertices[] = {
                // positions       
                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f,  0.5f, -0.5f,
                0.5f,  0.5f, -0.5f,
                -0.5f,  0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f, -0.5f,  0.5f,
                0.5f, -0.5f,  0.5f,
                0.5f,  0.5f,  0.5f,
                0.5f,  0.5f,  0.5f,
                -0.5f,  0.5f,  0.5f,
                -0.5f, -0.5f,  0.5f,

                -0.5f,  0.5f,  0.5f,
                -0.5f,  0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f,  0.5f,
                -0.5f,  0.5f,  0.5f,

                0.5f,  0.5f,  0.5f,
                0.5f,  0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f,  0.5f,
                0.5f,  0.5f,  0.5f,

                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f,  0.5f,
                0.5f, -0.5f,  0.5f,
                -0.5f, -0.5f,  0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f,  0.5f, -0.5f,
                0.5f,  0.5f, -0.5f,
                0.5f,  0.5f,  0.5f,
                0.5f,  0.5f,  0.5f,
                -0.5f,  0.5f,  0.5f,
                -0.5f,  0.5f, -0.5f
            };
            float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
            };

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path quadVertexPath = shaderFolder / "quadVertex.glsl";
            std::filesystem::path quadFragPath = shaderFolder / "quadFrag.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str()};
            LearnOpenGL::Shader quadShader{quadVertexPath.generic_string().c_str(), quadFragPath.generic_string().c_str()};
            cubeShader.use();

            enum ObjIndex
            {
                CUBE,
                QUAD,
            };

            // Arrays and Buffers
            unsigned int VAO[5];
            glGenVertexArrays(5, VAO);
            unsigned int VBO[5];
            glGenBuffers(5, VBO);

            // Cube
            glBindVertexArray(VAO[CUBE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, 0);
            glEnableVertexAttribArray(0);

            // Quad
            glBindVertexArray(VAO[QUAD]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[QUAD]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (void*)(sizeof(GL_FLOAT) * 2));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

            // Framebuffers
            unsigned int framebuffer;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

            // Generate target texture
            unsigned int targetTextureMS;
            glGenTextures(1, &targetTextureMS);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, targetTextureMS);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, targetTextureMS, 0);

            // Create stencil/depth renderbuffers
            unsigned int rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Intermidiate framebuffer
            unsigned int intermediateFBO;
            glGenFramebuffers(1, &intermediateFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

            unsigned int targetTexture;
            glGenTextures(1, &targetTexture);
            glBindTexture(GL_TEXTURE_2D, targetTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Uniforms
            quadShader.use();
            quadShader.setInt("screenTexture", 0);

            // Camera pos
            camera.Position = glm::vec3{0, 0, 3.f};

            // Game loop
            float delta = 0;
            while(!glfwWindowShouldClose(window))
            {   
                float now = glfwGetTime();
                // Window Input
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, true);

                // Clear
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Camera movement
                float cameraSpeed = 2.5f * delta;
                if(isKeyPressed(window, GLFW_KEY_W))
                    camera.ProcessKeyboard(Camera_Movement::FORWARD, delta);
                else if(isKeyPressed(window, GLFW_KEY_S))
                    camera.ProcessKeyboard(Camera_Movement::BACKWARD, delta);

                if(isKeyPressed(window, GLFW_KEY_A))
                    camera.ProcessKeyboard(Camera_Movement::LEFT, delta);
                else if(isKeyPressed(window, GLFW_KEY_D))
                    camera.ProcessKeyboard(Camera_Movement::RIGHT, delta);

                // Set uniforms
                auto projection = glm::perspective(glm::radians(camera.Zoom),  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.f);
                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 model{1.0f};
                glm::vec3 color = glm::vec3{1.0f} * (float)((sin(glfwGetTime()) + 1) / 2);
                color = glm::vec3{0.0f, 1.0f, 0.0f};
                cubeShader.use();
                cubeShader.setMatrix("projection", glm::value_ptr(projection));
                cubeShader.setMatrix("view", glm::value_ptr(view));
                cubeShader.setMatrix("model", glm::value_ptr(model));
                cubeShader.setVec3("color", glm::value_ptr(color));

                // Draw Cube
                cubeShader.use();
                glBindVertexArray(VAO[CUBE]);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Blit MS texture to intermediate
                glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
                glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

                // Draw quad
                quadShader.use();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindVertexArray(VAO[QUAD]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, targetTexture);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                // Events
                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);

                delta = glfwGetTime() - now;
                //std::cout << "FPS: " << 1/delta << "\n";
            }
        }
    }
    else
    {
        const char* error = new char[1024];
        glfwGetError(&error);

        std::cout << "Create window Error: " << error << '\n';
    }

    // Close
    glfwTerminate();
}