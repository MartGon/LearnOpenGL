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
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            // Models' vertices
            float vertices[] = {
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

                0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
            };
            float skyboxVertices[] = {
                // positions          
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
            };

            // Positions
            glm::vec3 cubePos[] = {
                glm::vec3(1.5f, 0.0f, 0.0f),
                glm::vec3(-1.5f, 0.0f, -1.0f)
            };

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path skyboxFragPath = shaderFolder / "skyboxFrag.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str() };
            LearnOpenGL::Shader skyShader{vertexPath.generic_string().c_str(), skyboxFragPath.generic_string().c_str() };
            cubeShader.use();

            // Textures - 2D
            std::filesystem::path texturesDir{TEXTURES_DIR};
            std::filesystem::path texturePath{texturesDir / "container.jpg"};
            auto texture = GenTexture(texturePath, GL_TEXTURE0, GL_RGB);
            cubeShader.setInt("Texture", 0);

            // Texture - Cubemap
            std::vector<std::filesystem::path> faces = {
                GetRelativePath(texturesDir, "right.jpg"),
                GetRelativePath(texturesDir, "left.jpg"),
                GetRelativePath(texturesDir, "top.jpg"),
                GetRelativePath(texturesDir, "bottom.jpg"),
                GetRelativePath(texturesDir, "front.jpg"),
                GetRelativePath(texturesDir, "back.jpg")
            };
            unsigned int cubemap = GenCubeMap(faces);
            skyShader.use();
            skyShader.setInt("Texture", 0);

            enum ObjIndex
            {
                CUBE,
                SKYBOX
            };

            // Arrays and Buffers
            unsigned int VAO[5];
            glGenVertexArrays(5, VAO);
            unsigned int VBO[5];
            glGenBuffers(5, VBO);

            // Cubes
            glBindVertexArray(VAO[CUBE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Skybox
            glBindVertexArray(VAO[SKYBOX]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[SKYBOX]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(0);

            // Model
            std::filesystem::path modelsDir{MODELS_DIR};
            std::filesystem::path modelPath{modelsDir / "backpack.obj"};
            LearnOpenGL::Model model{modelPath};

            // Set camera pos
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

                // Transformations
                auto view = camera.GetViewMatrix();                    
                auto projection = glm::perspective(glm::radians(camera.Zoom),  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.f);    
                cubeShader.use();
                cubeShader.setBool("skybox", false);
                cubeShader.setVec3("cameraPos", glm::value_ptr(camera.Position));
                cubeShader.setMatrix("view", glm::value_ptr(view));
                cubeShader.setMatrix("projection", glm::value_ptr(projection));

                // Cubes
                for(auto i = 0; i < 1; i++)
                {
                    cubeShader.use();
                    auto model = glm::translate(glm::mat4{1.0f}, cubePos[i]);
                    cubeShader.setMatrix("model", glm::value_ptr(model));
                    glBindVertexArray(VAO[CUBE]);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }

                // Model
                glm::mat4 modelMat = glm::translate(glm::mat4{1.0f}, cubePos[1]);
                cubeShader.setMatrix("model", glm::value_ptr(modelMat));
                model.Draw(cubeShader);

                // Skybox
                glm::mat4 model{1.0f};
                glm::mat4 skyView = glm::mat4(glm::mat3(view));
                
                skyShader.use();
                skyShader.setBool("skybox", true);
                skyShader.setMatrix("view", glm::value_ptr(skyView));
                skyShader.setMatrix("projection", glm::value_ptr(projection));
                skyShader.setMatrix("model", glm::value_ptr(model));
                glBindVertexArray(VAO[SKYBOX]);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                
                glfwPollEvents();
                
                // Swap buffers
                glfwSwapBuffers(window);

                delta = glfwGetTime() - now;
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