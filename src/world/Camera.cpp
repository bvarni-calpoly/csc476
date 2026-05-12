// FIXME DEPENDENCIES
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "../core/GLSL.h"
#include "../renderer/Program.h"
#include "../renderer/MatrixStack.h"
#include "../math/Bezier.h"
#include "../math/Spline.h"
#include "Camera.h"
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
void Camera::playerMovement(GLFWwindow *window, float maxSpeed, float deltaTime)
{
    float accel = 10.0f;
    float friction = 6.0f;
    float stopSpeed = 100.0f;
    float speedMult = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 2.0f : 1.0f;
    
    glm::vec3 wishDir = glm::vec3(0.0f);
    strafe = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    // 1. Movement Input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishDir += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishDir -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishDir -= strafe;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishDir += strafe;

    // 2. Jump Input (Ground only)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !airborne)
    {
        velocity.y = 250.0f; // Quake jump impulse is much higher than 80
        airborne = true;
    }

    // 3. Prepare wishDir/wishSpeed
    wishDir.y = 0; // Keep movement horizontal
    if (glm::length(wishDir) > 0) 
        wishDir = glm::normalize(wishDir);

    float wishSpeed = maxSpeed * speedMult;

    // 4. Ground Friction
    if (!airborne)
    {
        float speed = glm::length(velocity);
        if (speed > 0)
        {
            float control = (speed < stopSpeed) ? stopSpeed : speed;
            float drop = control * friction * deltaTime;
            float newSpeed = glm::max(speed - drop, 0.0f);
            velocity *= (newSpeed / speed);
        }
    }

    // 5. Acceleration (The Strafe-Jump logic)
    float currentSpeed = glm::dot(velocity, wishDir);
    float addSpeed = wishSpeed - currentSpeed;

    if (addSpeed > 0)
    {
        float accelSpeed = accel * deltaTime * wishSpeed;
        if (accelSpeed > addSpeed) accelSpeed = addSpeed;
        velocity += accelSpeed * wishDir;
    }

    // 6. Gravity (Always acting if in air)
    if (airborne)
    {
        velocity.y -= gravity * deltaTime;
    }

    // 7. FINAL POSITION UPDATE (Integrate velocity)
    eye += velocity * deltaTime;

    // 8. FLOOR COLLISION (The "Snap")
    // Do this AFTER moving the eye so the jump has a frame to clear the floor
    if (eye.y <= -5.0f)
    {
        eye.y = -5.0f;
        velocity.y = 0;
        airborne = false;
    }
    else
    {
        airborne = true;
    }
}

void Camera::SetView(std::shared_ptr<Program> shader)
{
	lookAtTarget = eye + forward; // FIXME <<<
	glm::mat4 Cam = glm::lookAt(eye, lookAtTarget, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, glm::value_ptr(Cam));
}

void Camera::SetPortalView(std::shared_ptr<Program> shader, std::shared_ptr<Camera> mainCamera, std::shared_ptr<MatrixStack> portalSource, std::shared_ptr<MatrixStack> portalDestination)
{
	//https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
	
	// Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
	//TD = TB^-1 * R * TA * TC
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)); // R, rotate 180 degrees
	glm::mat4 mainView = glm::lookAt(mainCamera->eye, mainCamera->lookAtTarget, glm::vec3(0, 1, 0)); // TC, lookAt returns view matrix
	glm::mat4 portalA = portalSource->topMatrix();	// TA
	glm::mat4 portalB = portalDestination->topMatrix();	// TB

	// 1. inverse(PortalB) - Move from world space -> destination local space
	// 2. Rotation		   - (optional) flip orientation 180 degrees
	// 3. Portal A		   - Move from rotated destination local space -> source world space
	// 4. mainView		   - Move from source world space -> camera space
	glm::mat4 virtualView = mainView * portalA * rotation * glm::inverse(portalB);

	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, glm::value_ptr(virtualView));
}

void Camera::SetRecursivePortalView(std::shared_ptr<Program> shader, std::shared_ptr<Camera> virtualCamera, std::shared_ptr<MatrixStack> portalSource, std::shared_ptr<MatrixStack> portalDestination)
{
	//https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
	
	// Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
	//TD = TB^-1 * R * TA * TC
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)); // R, rotate 180 degrees
	glm::mat4 mainView = glm::lookAt(virtualCamera->eye, virtualCamera->lookAtTarget, glm::vec3(0, 1, 0)); // TC, lookAt returns view matrix
	glm::mat4 portalA = portalSource->topMatrix();	// TA
	glm::mat4 portalB = portalDestination->topMatrix();	// TB

	// 1. inverse(PortalB) - Move from world space -> destination local space
	// 2. Rotation		   - (optional) flip orientation 180 degrees
	// 3. Portal A		   - Move from rotated destination local space -> source world space
	// 4. mainView		   - Move from source world space -> camera space
	glm::mat4 virtualView = mainView * portalA * rotation * glm::inverse(portalB);

	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, glm::value_ptr(virtualView));
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