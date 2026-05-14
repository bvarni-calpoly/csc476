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
#include "../physics/AABB.h"
#include "../SceneInitializer.h"
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

void Camera::playerMovement(GLFWwindow *window, std::shared_ptr<SceneInitializer> scene, float maxSpeed, float deltaTime)
{
	float accel = 10;
    //glm::vec3 accel = glm::vec3(0, -9.81/32, 0);
	float addSpeed, accelSpeed, currentSpeed, wishSpeed, speed, newSpeed, drop, control, speedMult;
	float stopSpeed = 100.0f, friction = 6.0f;
	
	glm::vec3 wishDir = glm::vec3(0.0f);
	strafe = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))); // get side basis vector (points right)
	up = normalize(glm::cross(forward, strafe));					  // get vertical basis vector (points up)

	// User input
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishDir += forward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishDir -= forward;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishDir -= strafe;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishDir += strafe;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) if(!airborne) velocity.y = 80;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speedMult = 2;
	else
		speedMult = 1;

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		velocity -= up * deltaTime * maxSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		velocity += up * deltaTime * maxSpeed;
	}

	// Normalize input
	if (glm::length(wishDir) > 0.001f)
		wishDir = glm::normalize(wishDir);

	wishSpeed = maxSpeed * speedMult;
	wishDir.y = 0;
	
	// Friction
	if(!airborne)
	{
		speed = glm::length(velocity);

		control = speed < stopSpeed ? stopSpeed : speed;
		drop += control * friction * deltaTime;

		newSpeed = glm::max(speed - drop, 0.0f);
		if (speed > 0) newSpeed /= speed;
		
		velocity *= newSpeed;
	}
	
	// Accelerate
	currentSpeed = glm::dot(velocity, wishDir);
	addSpeed = wishSpeed - currentSpeed;
	
	accelSpeed = accel * deltaTime * wishSpeed;

	// Limit max acceleration
	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;
	
	// physics updates
	velocity += accelSpeed * wishDir;

	velocity.y -= gravity * deltaTime;

	// if(eye.y > -5.0f)
	// {
	// 	//airborne = true;
	// 	velocity.y -= gravity * deltaTime;
	// }
	// else
	// {
	// 	airborne = false;
	// 	//velocity.y = 0;
	// 	eye.y = -5.0f;
	// }

	eye_prev = eye;

	// apply to player position
	eye += velocity * deltaTime;

	airborne = true;
    for (auto &mapGeomChild : scene->mapGeom->children)
    {
        //scene->mapGeom->collided = AABB::intersectsCamera(*scene->mainCamera, *mapGeomChild);
		//std::cout << mapGeomChild->objName << " " << mapGeomChild->portalID << std::endl;
		
		if (mapGeomChild->portalID > 0)
		{
			if (AABB::intersectsCameraPlane(*scene->mainCamera, *mapGeomChild))
			{
				if (mapGeomChild->portalID == 1)
				{
					std::cout << "Portal1" << std::endl;
					glm::vec3 offset = eye - (scene->portals[0]->position + glm::vec3(0, 0.0, 2.2f));
					eye = scene->portals[1]->position + offset;
					//eye = scene->portals[1]->position + glm::vec3(0.0f, scene->portals[1]->position.y / 2.0f, -1.5f);

				} else if (mapGeomChild->portalID == 2)
				{
					std::cout << "Portal2" << std::endl;
					glm::vec3 offset = eye - (scene->portals[1]->position + glm::vec3(0, 0, -2.2f));
					eye = scene->portals[0]->position + offset;
					//eye = scene->portals[0]->position + glm::vec3(0.0f, -scene->portals[0]->position.y / 2.0f, 1.5f);
				}
			}
		}
		else if (AABB::intersectsCamera(*scene->mainCamera, *mapGeomChild))
		{
			break;
		}
    }

	if (eye.y < -200.0f)
		eye = glm::vec3(0, 50.0f, 0);
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
	glm::mat4 portalA = portalSource->topMatrix();		// TA
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