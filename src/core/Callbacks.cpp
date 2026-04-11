// FIXME DEPENDENCIES
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "Callbacks.h"
#include "GLSL.h"
#include "../renderer/Program.h"
#include "../renderer/MatrixStack.h"
#include "../math/Bezier.h"
#include "../math/Spline.h"
#include "../world/Camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.1415927

Callbacks::Callbacks(Camera *cam): camera(cam) {}

Callbacks::~Callbacks()
{
}

void Callbacks::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    {
        mouseEnabled = !mouseEnabled;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        lightTrans += 0.5;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        lightTrans -= 0.5;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        mat = (mat + 1) % 2; // fixme
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_RELEASE)
    {
        freeCamera = !freeCamera;
    }
    if (key == GLFW_KEY_G && action == GLFW_RELEASE)
    {
        cinematicCamera = !cinematicCamera;
    }
}

void Callbacks::mouseCallback(GLFWwindow *window, int button, int action, int mods)
{    
    double posX, posY;

    if (action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &posX, &posY);
        std::cout << "Pos X " << posX << " Pos Y " << posY << std::endl;

        mouseEnabled = !mouseEnabled;
        std::cout << "Mouse toggle:" << mouseEnabled << std::endl;
    }
}

void Callbacks::scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
{
    std::cout << "xDel + yDel " << deltaX << " " << deltaY << std::endl;
}

// https://www.glfw.org/docs/latest/input_guide.html#cursor_pos
// https://learnopengl.com/Getting-started/Camera
// https://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
void Callbacks::setCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    double xsensitivity = 0.01;
    double ysensitivity = 0.005;

    phi += yoffset * ysensitivity;   // pitch
    theta += xoffset * xsensitivity; // yaw

    phi = glm::clamp(phi, -PI / 2.0 + 0.1, PI / 2.0 - 0.1); // 180 degrees front view

    glm::vec3 direction = glm::vec3(
        cos(theta) * cos(phi), // x
        sin(phi),              // y
        sin(theta) * cos(phi)  // z
    );

    // change direction the camera is looking at so the camera moves towards this vector
    camera->forward = glm::normalize(direction);

    camera->lookAtTarget = camera->eye + camera->forward;
}

void Callbacks::resizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}