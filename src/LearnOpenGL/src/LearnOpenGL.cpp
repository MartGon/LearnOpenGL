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
#include <Camera.h>

unsigned int GenTexture(std::filesystem::path path, int textureUnit = GL_TEXTURE0, int format = GL_RGB, int glFormat = GL_RGB)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    auto data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

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

enum ObjIndex
{
    CUBE,
    LIGHT,
    PLANE,
    QUAD
};

void DrawScene(glm::vec3* cubePos, unsigned int* VAO, LearnOpenGL::Shader& shader, unsigned int texture, unsigned int specularMap, unsigned int wood)
{
    // Draw Cubes
    for(unsigned int i = 0; i < 3; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePos[i]);
        shader.use();
        shader.setMatrix("model", glm::value_ptr(model));

        // Draw
        // Note: This triggers a segfault if the VerterAttribPointer of a in var is not defined
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glBindVertexArray(VAO[CUBE]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Draw Floor
    glm::mat4 floorModel{1.0f};
    shader.use();
    shader.setMatrix("model", glm::value_ptr(floorModel));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wood);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, wood);
    glBindVertexArray(VAO[PLANE]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
            glm::vec3 cubePos[] = {
                glm::vec3(0.0f, 1.5f, 0.0),
                glm::vec3(2.0f, 0.0f, 1.0),
                glm::vec3(-1.0f, 0.0f, 2.0)
            };
            glm::vec3 pointLightPositions[] = {
                glm::vec3( -2.0f, 4.0f, -1.0f),
                glm::vec3( 2.3f, -3.3f, -4.0f),
                glm::vec3(-4.0f,  2.0f, -12.0f),
                glm::vec3( 0.0f,  0.0f, -3.0f)
            };
            float planeVertices[] = 
            {
                25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
                -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
                -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

                25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
                -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
                25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 10.0f
            };
            float quadVertices[] = 
            {
                -1.0f, -1.0f,   0.0f, 0.0f,
                1.0f, -1.0f,    1.0f, 0.0f,
                1.0f, 1.0f,     1.0f, 1.0f,
                1.0f, 1.0f,     1.0f, 1.0f,
                -1.0f, 1.0f,    0.0f, 1.0f,
                -1.0f, -1.0f,   0.0f, 0.0f,

            };

            // Shadow maps - Framebuffer
            unsigned int depthMapFBO;
            glGenFramebuffers(1, &depthMapFBO);
            const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

            // Shadow map - Depth Map
            unsigned int depthMap;
            glGenTextures(1, &depthMap);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Shadow maps - Framebuffer attachments
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Shader program
            std::filesystem::path shaderFolder{SHADERS_DIR};
            std::filesystem::path vertexPath = shaderFolder / "vertex.glsl";
            std::filesystem::path cubeFragPath = shaderFolder / "cubeFrag.glsl";
            std::filesystem::path lightFragPath = shaderFolder / "lightFrag.glsl";
            std::filesystem::path shadowVertexPath = shaderFolder / "shadowVertex.glsl";
            std::filesystem::path shadowFragPath = shaderFolder / "shadowFrag.glsl";
            std::filesystem::path quadVertexPath = shaderFolder / "quadVertex.glsl";
            std::filesystem::path quadFragPath = shaderFolder / "quadFrag.glsl";
            LearnOpenGL::Shader cubeShader{vertexPath.generic_string().c_str(), cubeFragPath.generic_string().c_str() };
            LearnOpenGL::Shader lightShader{vertexPath.generic_string().c_str(), lightFragPath.generic_string().c_str() };
            LearnOpenGL::Shader shadowShader{shadowVertexPath.generic_string().c_str(), shadowFragPath.generic_string().c_str() };
            LearnOpenGL::Shader quadShader{quadVertexPath.generic_string().c_str(), quadFragPath.generic_string().c_str() };
            cubeShader.use();

            // Arrays and Buffers
            unsigned int VAO[4];
            glGenVertexArrays(4, VAO);
            unsigned int VBO[4];
            glGenBuffers(4, VBO);

            // Cube
            glBindVertexArray(VAO[CUBE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // Plane
            glBindVertexArray(VAO[PLANE]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[PLANE]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
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

            // Screen quad
            glBindVertexArray(VAO[QUAD]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[QUAD]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);


            // Textures
            std::filesystem::path texturesDir{TEXTURES_DIR};
            std::filesystem::path texturePath = texturesDir / "container2.png";
            auto texture = GenTexture(texturePath, GL_TEXTURE0, GL_RGBA, GL_SRGB_ALPHA);

            std::filesystem::path specularPath = texturesDir / "container2_specular.png";
            auto specularMap = GenTexture(specularPath, GL_TEXTURE1, GL_RGBA);

            std::filesystem::path woodPath = texturesDir / "wood.png";
            auto wood = GenTexture(woodPath, GL_TEXTURE0, GL_RGBA, GL_SRGB_ALPHA);

            // Set material properties
            cubeShader.use();
            cubeShader.setInt("material.diffuse", 0);
            cubeShader.setInt("material.specular", 1);
            cubeShader.setFloat("material.shininess", 8.0f);

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

            // Shadow Maps - Ortographic view
            float zNear = 1.0, zFar = 7.5f;
            glm::mat4 ortho = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
            glm::mat4 lightView = glm::lookAt(pointLightPositions[0], glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 lightSpaceMat = ortho * lightView;
            shadowShader.use();
            shadowShader.setMatrix("lightSpaceMatrix", glm::value_ptr(lightSpaceMat));

            cubeShader.use();
            cubeShader.setMatrix("lightSpaceMatrix", glm::value_ptr(lightSpaceMat));
            cubeShader.setInt("shadowMap", 2);
            
            // Light flags
            bool lightsOn[] = {false, false, false, false};
            bool sun = false;
            bool flashlight = false;
            bool blinn = true;

            // Game loop
            float delta = 0;
            while(!glfwWindowShouldClose(window))
            {   
                float now = glfwGetTime();
                // Window Input
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, true);

                // Draw Shadows - BEGIN
                glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                
                DrawScene(cubePos, VAO, shadowShader, texture, specularMap, wood);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                // Draw Shadows - END
                // Draw Scene - BEGIN
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

                // Clear
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, depthMap);

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

                if(isKeyPressed(window, GLFW_KEY_G))
                    camera.Position = pointLightPositions[0];
                else if(isKeyPressed(window, GLFW_KEY_O))
                    camera.Position = glm::vec3(0.0f);

                // Camera pos
                cubeShader.use();
                cubeShader.setVec3("viewPos", glm::value_ptr(camera.Position));
                cubeShader.setVec3("spotLight.pos", glm::value_ptr(camera.Position));
                cubeShader.setVec3("spotLight.dir", glm::value_ptr(camera.Front));

                // Transformations
                auto view = camera.GetViewMatrix();                    
                auto projection = glm::perspective(glm::radians(camera.Zoom),  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.f);
                cubeShader.use();
                cubeShader.setMatrix("projection", glm::value_ptr(projection));
                cubeShader.setMatrix("view", glm::value_ptr(view));    

                // Light
                lightShader.use();
                lightShader.setMatrix("view", glm::value_ptr(view));
                lightShader.setMatrix("projection", glm::value_ptr(projection));

                for(auto i = 0; i < 4; i++)
                {
                    auto model = glm::translate(glm::mat4{1.0f}, pointLightPositions[i]);
                    model = glm::scale(model, glm::vec3(0.2f)); 
                    lightShader.setMatrix("model", glm::value_ptr(model));

                    glBindVertexArray(VAO[LIGHT]);
                    if(lightsOn[i])
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }

                // Light Control
                cubeShader.use();
                if(isKeyPressed(window, GLFW_KEY_K))
                    sun = !sun;
                cubeShader.setBool("sunOn", sun);
                if(isKeyPressed(window, GLFW_KEY_L))
                    flashlight = !flashlight;
                cubeShader.setBool("flashlightOn", flashlight);
                if(isKeyPressed(window, GLFW_KEY_B))
                    blinn = !blinn;
                cubeShader.setBool("blinn", blinn);

                for(auto i = 0; i < 4; i++)
                {   
                    if(isKeyPressed(window, GLFW_KEY_1 + i))
                        lightsOn[i] = !lightsOn[i];
                    cubeShader.setBool("lightsOn["+ std::to_string(i) + "]", lightsOn[i]);
                }

                DrawScene(cubePos, VAO, cubeShader, texture, specularMap, wood);
                // Draw Scene - END

                quadShader.use();
                quadShader.setInt("iTexture", 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depthMap);
                glBindVertexArray(VAO[QUAD]);
                //glDrawArrays(GL_TRIANGLES, 0, 6);

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