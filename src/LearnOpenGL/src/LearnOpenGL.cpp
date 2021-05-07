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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
            glEnable(GL_BLEND);   
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

            // Models' vertices
            float vertices[] = {
                // Back face
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                // Front face
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                // Left face
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                // Right face
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
                // Bottom face
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                // Top face
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left   
            };
            float quad3dPos[] = {
                -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
                -0.5f, 0.5f, 0.0f,   0.0f, 1.0f,
                0.5f, 0.5f, 0.0f,   1.0f, 1.0f,
                0.5f, 0.5f, 0.0f,   1.0f, 1.0f,
                0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
                -0.5f, -0.5f, 0.0f,   0.0f, 0.0f
            };
            float quad[] = {
                -0.5f, -0.5f,   0.0f, 0.0f,
                -0.5f, 0.5f,    0.0f, 1.0f,
                0.5f, 0.5f,    1.0f, 1.0f,
                0.5f, 0.5f,    1.0f, 1.0f,
                0.5f, -0.5f,   1.0f, 0.0f,
                -0.5f, -0.5f,   0.0f, 0.0f
            };

            // Positions
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
                glm::vec3(1.5f, 0.0f, 0.0f),
                glm::vec3(-1.5f, 0.0f, -1.0f)
            };
            std::vector<glm::vec3> grassPos = {
                glm::vec3(-1.5f,  0.0f, -0.48f),
                glm::vec3( 1.5f,  0.0f,  0.51f),
                glm::vec3( 0.0f,  0.0f,  0.7f),
                glm::vec3(-0.3f,  0.0f, -2.3f),
                glm::vec3( 0.5f,  0.0f, -0.6f)
            };

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path lightFragPath = shaderFolder / "lightFrag.glsl";
            std::filesystem::path quadVertexPath = shaderFolder / "quadVertex.glsl";
            std::filesystem::path quadFragPath = shaderFolder / "quadFrag.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str() };
            LearnOpenGL::Shader lightShader{vertexPath.generic_string().c_str(), lightFragPath.generic_string().c_str() };
            LearnOpenGL::Shader quadShader{quadVertexPath.generic_string().c_str(), quadFragPath.generic_string().c_str() };
            quadShader.use();
            quadShader.setInt("screenTexture", 0);
            cubeShader.use();

            enum ObjIndex
            {
                PLANE,
                CUBE,
                LIGHT,
                QUAD,
                QUAD_3DPOS
            };

            // FrameBuffer
            unsigned int framebuffer;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

            // Generate target texture
            unsigned int targetTexture;
            glGenTextures(1, &targetTexture);
            glBindTexture(GL_TEXTURE_2D, targetTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);

            // RenderBuffer for Stencil and Depth testing
            unsigned int renderbuffer;
            glGenRenderbuffers(1, &renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "Framebuffer is not complete. ABORTING!!!\n";
                return -1;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Arrays and Buffers
            unsigned int VAO[5];
            glGenVertexArrays(5, VAO);
            unsigned int VBO[5];
            glGenBuffers(5, VBO);

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

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Light source
            glBindVertexArray(VAO[LIGHT]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[LIGHT]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);

            // Quad
            glBindVertexArray(VAO[QUAD_3DPOS]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[QUAD_3DPOS]);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glBufferData(GL_ARRAY_BUFFER, sizeof(quad3dPos), quad3dPos, GL_STATIC_DRAW);

            // Quad 3D Pos
            glBindVertexArray(VAO[QUAD]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[QUAD]);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

            // Textures
            std::filesystem::path texturesDir{TEXTURES_DIR};
            std::filesystem::path grassTexturePath{texturesDir / "blending_transparent_window.png"};
            stbi_set_flip_vertically_on_load(true);
            auto texture = GenTexture(grassTexturePath, GL_TEXTURE0, GL_RGBA);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            cubeShader.setInt("material.texture_diffuse1", 0);

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

                // Bind
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

                // Clear
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

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

                // Floor
                glBindVertexArray(VAO[PLANE]);
                glm::mat4 model{1.0f};
                glm::vec3 color{0.5f};
                lightShader.use();
                lightShader.setMatrix("model", glm::value_ptr(model));
                lightShader.setVec3("color", glm::value_ptr(color));
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Lights
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

                // Cubes
                glEnable(GL_CULL_FACE);
                for(auto i = 0; i < 2; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, cubePos[i]);
                    lightShader.setMatrix("model", glm::value_ptr(model));
                    auto color = glm::vec3{1.0f, 1.0f, 0.0f};
                    lightShader.setVec3("color", glm::value_ptr(color));
                    glBindVertexArray(VAO[CUBE]);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                glDisable(GL_CULL_FACE);

                // Quads
                auto sort = [&view](const glm::vec3& a, const glm::vec3& b)
                {
                    auto tA = glm::vec4(a, 1) * view;
                    auto tB = glm::vec4(b, 1) * view;

                    return tA.z < tB.z;
                };
                std::sort(grassPos.begin(), grassPos.end(), sort);

                glBindTexture(GL_TEXTURE_2D, texture);
                glEnable(GL_DEPTH_TEST);
                for(auto i = 0; i < 5; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, grassPos[i]);
                    cubeShader.use();
                    cubeShader.setMatrix("model", glm::value_ptr(model));
                    glBindVertexArray(VAO[QUAD_3DPOS]);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }

                // Draw quad with the framebuffer texture 
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                quadShader.use();
                glBindVertexArray(VAO[QUAD]);
                glDisable(GL_DEPTH_TEST);
                glBindTexture(GL_TEXTURE_2D, targetTexture);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                
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