// FIXME DEPENDENCIES
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "math/Bezier.h"
#include "math/Spline.h"
#include "physics/Camera.h"
#include "physics/Callbacks.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.1415927

Callbacks::Callbacks(/* args */)
{
}

Callbacks::~Callbacks()
{
}

void Callbacks::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        lightTrans += 0.5;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
        lightTrans -= 0.5;
    }
    //toggle material
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        g_Mat = (g_Mat + 1) % 3;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
        goCamera = !goCamera;
    }
}

void Callbacks::mouseCallback(GLFWwindow *window, int button, int action, int mods)
{
    double posX, posY;

    if (action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &posX, &posY);
        cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
    }
}

void Callbacks::scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
    cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
}

// https://www.glfw.org/docs/latest/input_guide.html#cursor_pos
// https://learnopengl.com/Getting-started/Camera
// https://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
void Callbacks::setCursorPosCallback(GLFWwindow* window,  double xpos, double ypos)
{
    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    double xsensitivity = 0.01;
    double ysensitivity = 0.005;

    phi	+= yoffset * ysensitivity; // pitch
    theta	+= xoffset * xsensitivity; // yaw

    phi = glm::clamp(phi, -PI/2.0 + 0.1, PI/2.0 - 0.1); // 180 degrees front view
    
    glm::vec3 direction = glm::vec3(
        cos(theta)*cos(phi),	// x
        sin(phi),					// y
        sin(theta)*cos(phi)		// z
    );

    // change direction the camera is looking at so the camera moves towards this vector
    forward = glm::normalize(direction);

    lookAtTarget = eye + forward;
}

void Callbacks::resizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}