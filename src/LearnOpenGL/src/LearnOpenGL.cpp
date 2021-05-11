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
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            // Model
            stbi_set_flip_vertically_on_load(true);
            std::filesystem::path modelsDir{MODELS_DIR};
            std::filesystem::path planetPath{modelsDir / "planet.obj"};
            std::filesystem::path asteroidPath{modelsDir / "rock.obj"};
            LearnOpenGL::Model planet{planetPath};
            LearnOpenGL::Model asteroid{asteroidPath};

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path planetVertexPath = shaderFolder / "planetVertex.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str()};
            LearnOpenGL::Shader planetShader{planetVertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str()};
            cubeShader.use();

            // Transformations
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_real_distribution<float> scaleDist(0.05, 0.25f);
            std::uniform_real_distribution<float> heightDist(0, 6.f);
            std::uniform_real_distribution<float> factorDist(0.15f, 1.0f);
            std::uniform_real_distribution<float> angleDist(0, 2 * glm::two_pi<float>());

            constexpr int amount = 100000;
            glm::mat4 transformations[amount];
            const float radius = 150.0f;
            for(int i = 0; i < amount; i++)
            {
                // Random offset
                auto scale = scaleDist(rng);
                auto factor = factorDist(rng);
                auto height = heightDist(rng);
                auto angle = angleDist(rng);

                glm::vec3 r = glm::vec3{1.0f, 0.0f, 0.0f}  * radius * factor;
                glm::mat4 transform{1.0f};
                transform = glm::rotate(transform, angle, glm::vec3(0, 1.0f, 0.0f));
                transform = glm::translate(transform, r);
                transform = glm::translate(transform, glm::vec3(0.0, height, 0.0f));
                transform = glm::rotate(transform, angleDist(rng), glm::vec3(0.4f, 0.6f, 0.8f));
                transform = glm::scale(transform, glm::vec3{scale});
                transformations[i] = transform;
            }

            enum ObjIndex
            {
                QUAD,
                TRANSLATIONS
            };

            // Arrays and Buffers
            unsigned int VAO[5];
            glGenVertexArrays(5, VAO);
            unsigned int VBO[5];
            glGenBuffers(5, VBO);

            // Translations
            glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSLATIONS]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amount, &transformations[0], GL_STATIC_DRAW);
            //glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            for(unsigned int i = 0; i < asteroid.meshes.size(); i++)
            {
                glBindVertexArray(asteroid.meshes[i].VAO);

                glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 * sizeof(glm::vec4)));
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
                glEnableVertexAttribArray(6);

                glVertexAttribDivisor(3, 1);
                glVertexAttribDivisor(4, 1);
                glVertexAttribDivisor(5, 1);
                glVertexAttribDivisor(6, 1);

                glBindVertexArray(0);
            }
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
                cubeShader.use();
                cubeShader.setMatrix("projection", glm::value_ptr(projection));
                cubeShader.setMatrix("view", glm::value_ptr(view));

                planetShader.use();
                planetShader.setMatrix("projection", glm::value_ptr(projection));
                planetShader.setMatrix("view", glm::value_ptr(view));
                
                // Draw planet
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
                model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
                planetShader.use();
                planetShader.setMatrix("model", glm::value_ptr(model));
                planet.Draw(planetShader);

                // Draw Asteroids
                cubeShader.use();
                cubeShader.setInt("texture_diffuse1", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, asteroid.textures_loaded[0].id);
                for(unsigned int i = 0; i < asteroid.meshes.size(); i++)
                {
                    glBindVertexArray(asteroid.meshes[i].VAO);
                    glDrawElementsInstanced(GL_TRIANGLES, asteroid.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
                    glBindVertexArray(0);
                }
                


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