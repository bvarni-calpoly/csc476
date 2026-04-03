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
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(/* args */)
{
}

Camera::~Camera()
{
}

// https://learnopengl.com/Getting-started/Camera
void Camera::cameraMovement(GLFWwindow *window, float cameraSpeed, float deltaTime)
{
	strafe = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))); // get side basis vector (points right)
	up = normalize(glm::cross(forward, strafe));					  // get vertical basis vector (points up)

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		eye += forward * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		eye -= forward * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		eye -= strafe * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		eye += strafe * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		eye += glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		eye -= glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		eye -= up * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		eye += up * deltaTime * cameraSpeed;
	}
}

void Camera::playerMovement(GLFWwindow *window, float cameraSpeed, float deltaTime)
{
	// fixme
}

void Camera::SetView(std::shared_ptr<Program> shader)
{
	lookAtTarget = eye + forward; // FIXME <<<
	glm::mat4 Cam = glm::lookAt(eye, lookAtTarget, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, glm::value_ptr(Cam));
}

void Camera::updateUsingCameraPath(float frametime, Spline *splinepath)
{
	if (!splinepath[0].isDone())
	{
		splinepath[0].update(frametime);
		eye = splinepath[0].getPosition();
	}
	else if (!splinepath[1].isDone())
	{
		splinepath[1].update(frametime);
		eye = splinepath[1].getPosition();
	}
	else if (!splinepath[2].isDone())
	{
		splinepath[2].update(frametime);
		eye = splinepath[2].getPosition();
	}
	else
	{
		splinepath[3].update(frametime);
		eye = splinepath[3].getPosition();
	}
}