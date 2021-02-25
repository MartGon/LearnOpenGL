#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main()
{
    std::cout << "Hello world!\n";

    glfwInit();
    auto window = glfwCreateWindow(640, 480, "Window", NULL, NULL);

    const char* error;
    glfwGetError(&error);
    std::cout << error << "\n";
    glfwShowWindow(window);

    std::string input;
    std::cin >> input;
}