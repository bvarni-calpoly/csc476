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
	g_strafe = glm::normalize(glm::cross(g_forward, glm::vec3(0, 1, 0))); // get side basis vector (points right)
	g_up = normalize(glm::cross(g_forward, g_strafe));					  // get vertical basis vector (points up)

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_eye += g_forward * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_eye -= g_forward * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_eye -= g_strafe * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_eye += g_strafe * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		g_eye += glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		g_eye -= glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		g_eye -= g_up * deltaTime * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		g_eye += g_up * deltaTime * cameraSpeed;
	}
}

void Camera::playerMovement(GLFWwindow *window, float cameraSpeed, float deltaTime)
{
	// fixme
}

void Camera::SetView(std::shared_ptr<Program> shader)
{
	// glm::vec3 g_lookAt = g_eye + g_forward; FIXME
	glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, glm::value_ptr(Cam));
}

void Camera::updateUsingCameraPath(float frametime, Spline *splinepath)
{
	if (!splinepath[0].isDone())
	{
		splinepath[0].update(frametime);
		g_eye = splinepath[0].getPosition();
	}
	else if (!splinepath[1].isDone())
	{
		splinepath[1].update(frametime);
		g_eye = splinepath[1].getPosition();
	}
	else if (!splinepath[2].isDone())
	{
		splinepath[2].update(frametime);
		g_eye = splinepath[2].getPosition();
	}
	else
	{
		splinepath[3].update(frametime);
		g_eye = splinepath[3].getPosition();
	}
}