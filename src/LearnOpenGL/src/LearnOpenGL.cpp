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

unsigned int GenTexture(std::filesystem::path path, int textureUnit = GL_TEXTURE0, int format = GL_RGB)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    auto data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;

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
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            float vertices[] = {
                // positions          // normals           // texture coords
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
                0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

                0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
            };     
            float planeVertices[] = {
                // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
                5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
                -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
                -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

                5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
                -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
                5.0f, -0.5f, -5.0f,  2.0f, 2.0f								
            };
            glm::vec3 pointLightPositions[] = {
                glm::vec3( 0.7f,  0.2f,  2.0f),
                glm::vec3( 2.3f, -3.3f, -4.0f),
                glm::vec3(-4.0f,  2.0f, -12.0f),
                glm::vec3( 0.0f,  0.0f, -3.0f)
            };
            glm::vec3 cubePos[] = {
                glm::vec3(2.0f, 0.0f, 0.0f),
                glm::vec3(-1.0f, 0.0f, -1.0f)
            };

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path lightFragPath = shaderFolder / "lightFrag.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str() };
            LearnOpenGL::Shader lightShader{vertexPath.generic_string().c_str(), lightFragPath.generic_string().c_str() };
            cubeShader.use();

            enum ObjIndex
            {
                PLANE,
                CUBE,
                LIGHT
            };

            // Arrays and Buffers
            unsigned int VAO[3];
            glGenVertexArrays(3, VAO);
            unsigned int VBO[2];
            glGenBuffers(2, VBO);

            // Plane
            glBindVertexArray(VAO[PLANE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[PLANE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Cubes
            glBindVertexArray(VAO[CUBE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // Light source
            glBindVertexArray(VAO[LIGHT]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
            
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);

            // Light colors
            glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };
            glm::vec3 lightAmbient = lightColor * glm::vec3{ 0.05f };
            glm::vec3 lightDiffuse = lightColor * glm::vec3{ 0.5f };
            glm::vec3 lightSpecular{ 1.0f };

            // Direcitonal Light
            glm::vec3 lightDir{-0.2f, -1.0f, -0.3f};
            glm::vec3 dirLightAmbient = lightColor * glm::vec3{ 0.05f };
            glm::vec3 dirLightDiffuse = lightColor * glm::vec3{ 0.4f };
            glm::vec3 dirLightSpecular = lightColor * glm::vec3{ 0.5f };
            cubeShader.setVec3("dirLight.dir", glm::value_ptr(lightDir));
            cubeShader.setVec3("dirLight.light.ambient", glm::value_ptr(dirLightAmbient));
            cubeShader.setVec3("dirLight.light.diffuse", glm::value_ptr(dirLightDiffuse));
            cubeShader.setVec3("dirLight.light.specular", glm::value_ptr(dirLightSpecular));

            // PointLights
            glm::vec3 pointLightAmbient = lightColor * glm::vec3{ 0.05f };
            glm::vec3 pointLightDiffuse = lightColor * glm::vec3{ 0.8f };
            glm::vec3 pointLightSpecular = lightColor * glm::vec3{ 1.f };
            float constant = 1.0f;
            float linear = 0.09;
            float quadratic = 0.032;
            for(auto i = 0; i < 4; i++)
            {
                std::string pointLight = "pointLights[" + std::to_string(i) + "]";
                cubeShader.setVec3(pointLight + ".pos", glm::value_ptr(pointLightPositions[i]));
                cubeShader.setVec3(pointLight + ".light.ambient", glm::value_ptr(pointLightAmbient));
                cubeShader.setVec3(pointLight + ".light.diffuse", glm::value_ptr(pointLightDiffuse));
                cubeShader.setVec3(pointLight + ".light.specular", glm::value_ptr(pointLightSpecular));
                cubeShader.setFloat(pointLight + ".attenuation.constant", constant);
                cubeShader.setFloat(pointLight + ".attenuation.linear", linear);
                cubeShader.setFloat(pointLight + ".attenuation.quadratic", quadratic);
            }

            // Flashlight
            glm::vec3 spotLightAmbient = lightColor * glm::vec3{ 0.0f };
            glm::vec3 spotLightDiffuse = lightColor * glm::vec3{ 1.f };
            glm::vec3 spotLightSpecular = lightColor * glm::vec3{ 1.f };
            float slConstant = 1.0f;
            float slLinear = 0.09;
            float slQuadratic = 0.032;
            cubeShader.setVec3("spotLight.light.ambient", glm::value_ptr(spotLightAmbient));
            cubeShader.setVec3("spotLight.light.diffuse", glm::value_ptr(spotLightDiffuse));
            cubeShader.setVec3("spotLight.light.specular", glm::value_ptr(spotLightSpecular));
            cubeShader.setFloat("spotLight.attenuation.constant", slConstant);
            cubeShader.setFloat("spotLight.attenuation.linear", slLinear);
            cubeShader.setFloat("spotLight.attenuation.quadratic", slQuadratic);
            cubeShader.setFloat("spotLight.iCutOff", glm::cos(glm::radians(6.5f)));
            cubeShader.setFloat("spotLight.oCutOff", glm::cos(glm::radians(12.0f)));

            // Set camera pos
            camera.Position = glm::vec3{0, 0, 3.f};
            
            // Light flags
            bool lightsOn[] = {false, false, false, false};
            bool sun = true;
            bool flashlight = true;

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
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

                // Camera pos
                cubeShader.use();
                cubeShader.setVec3("viewPos", glm::value_ptr(camera.Position));
                cubeShader.setVec3("spotLight.pos", glm::value_ptr(camera.Position));
                cubeShader.setVec3("spotLight.dir", glm::value_ptr(camera.Front));

                // Light Control
                cubeShader.use();
                if(isKeyPressed(window, GLFW_KEY_K))
                    sun = !sun;
                cubeShader.setBool("sunOn", sun);
                if(isKeyPressed(window, GLFW_KEY_L))
                    flashlight = !flashlight;
                cubeShader.setBool("flashlightOn", flashlight);

                for(auto i = 0; i < 4; i++)
                {   
                    if(isKeyPressed(window, GLFW_KEY_1 + i))
                        lightsOn[i] = !lightsOn[i];
                    cubeShader.setBool("lightsOn["+ std::to_string(i) + "]", lightsOn[i]);
                }

                // Transformations
                auto view = camera.GetViewMatrix();                    
                auto projection = glm::perspective(glm::radians(camera.Zoom),  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.f);    

                // Model
                glm::mat4 pos{1.0f};
                pos = glm::translate(pos, glm::vec3{0.0f, 0.0f, 0.0f});
                pos = glm::scale(pos, glm::vec3(1.0f, 1.0f, 1.0f));

                cubeShader.use();
                cubeShader.setMatrix("view", glm::value_ptr(view));
                cubeShader.setMatrix("projection", glm::value_ptr(projection));
                cubeShader.setMatrix("model", glm::value_ptr(pos));

                // Light
                lightShader.use();
                lightShader.setMatrix("view", glm::value_ptr(view));
                lightShader.setMatrix("projection", glm::value_ptr(projection));

                for(auto i = 0; i < 4; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, pointLightPositions[i]);
                    model = glm::scale(model, glm::vec3(0.2f)); 
                    auto white = glm::vec3(1.0f);
                    lightShader.setMatrix("model", glm::value_ptr(model));
                    lightShader.setVec3("color", glm::value_ptr(white));

                    glBindVertexArray(VAO[LIGHT]);
                    if(lightsOn[i])
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                }

                // Floor           
                glStencilMask(0x00);
                glBindVertexArray(VAO[PLANE]);
                glm::mat4 model{1.0f};
                glm::vec3 color{0.5f};
                lightShader.setMatrix("model", glm::value_ptr(model));
                lightShader.setVec3("color", glm::value_ptr(color));
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Cubes
                std::vector<glm::vec3> cubePosVec{cubePos[0], cubePos[1]};
                auto cameraPos = camera.Position;
                std::sort(cubePosVec.begin(), cubePosVec.end(), [&cameraPos](const glm::vec3& a, const glm::vec3& b){
                    auto distA = glm::length(a - cameraPos);
                    auto distB = glm::length(b - cameraPos);

                    return distA < distB;
                });
                for(auto i = 0; i < 2; i++)
                {
                    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always writes over buffer values a 1 
                    glStencilMask(0xFF); // The value to be written is unchanged
                    glEnable(GL_DEPTH_TEST);

                    // Draw cube
                    auto model = glm::translate(glm::mat4{1.0f}, cubePosVec[i]);
                    lightShader.setMatrix("model", glm::value_ptr(model));
                    auto color = glm::vec3{0.3f, 0.5f, (float)i};
                    lightShader.setVec3("color", glm::value_ptr(color));
                    glBindVertexArray(VAO[CUBE]);
                    glDrawArrays(GL_TRIANGLES, 0, 36);

                    // Draw outline
                    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // Accept values in buffer positions that are not 1
                    glStencilMask(0x00); // Doens't override stencil buffer
                    //glDisable(GL_DEPTH_TEST);
                    
                    glm::vec3 scale{1.0f, 1.0f, 1.0f};
                    color = glm::vec3{1.0f, 1.0f, (float)i};
                    model = glm::scale(model, scale * 1.1f);
                    lightShader.setMatrix("model", glm::value_ptr(model));
                    lightShader.setVec3("color", glm::value_ptr(color));
                    glBindVertexArray(VAO[CUBE]);
                    glDrawArrays(GL_TRIANGLES, 0, 36);

                    glClear(GL_STENCIL_BUFFER_BIT);
                }

                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
                glEnable(GL_DEPTH_TEST);

                /*
                // Cubes' Outline
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilMask(0x00);
                glDisable(GL_DEPTH_TEST);
                for(auto i = 0; i < 2; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, cubePos[i]);
                    glm::vec3 scale{1.0f, 1.0f, 1.0f};
                    model = glm::scale(model, scale * 1.1f);
                    lightShader.setMatrix("model", glm::value_ptr(model));
                    auto color = glm::vec3{1.f, 1.f, 1.f};
                    lightShader.setVec3("color", glm::value_ptr(color));
                    glBindVertexArray(VAO[CUBE]);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }

                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
                glEnable(GL_DEPTH_TEST);
                */

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