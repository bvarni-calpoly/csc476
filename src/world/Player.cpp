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
#include "../SceneRender.h"
#include "Camera.h"
#include "Player.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Player::playerMovement(GLFWwindow *window, std::shared_ptr<SceneInitializer> &scene, float maxSpeed, float deltaTime)
{
    // Camera data
    glm::vec3 &up = scene->mainCamera->up;
    glm::vec3 &strafe = scene->mainCamera->strafe;
    glm::vec3 &forward = scene->mainCamera->forward;
    glm::vec3 &eye = scene->mainCamera->eye;
    glm::vec3 &eye_prev = scene->mainCamera->eye_prev;

    float accel = 10;
    // glm::vec3 accel = glm::vec3(0, -9.81/32, 0);
    float addSpeed, accelSpeed, currentSpeed, wishSpeed, speed, newSpeed, drop = 0.0f, control, speedMult = 1.0f;
    float stopSpeed = 100.0f, friction = 6.0f;

    glm::vec3 wishDir = glm::vec3(0.0f);
    strafe = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))); // get side basis vector (points right)
    up = normalize(glm::cross(forward, strafe));                      // get vertical basis vector (points up)

    // User input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        wishDir += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        wishDir -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        wishDir -= strafe;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        wishDir += strafe;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        if (!airborne)
            velocity.y = 150;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speedMult = 2;
    else
        speedMult = 1;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        velocity -= up * deltaTime * maxSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        velocity += up * deltaTime * maxSpeed;
    }

    // Projectile
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!scene->projectile->active)
        {
            scene->projectile->active = true; // projectile spawned
            float rocketSpeed = 300.0f;
            float forwardOffset = 1.5f;
            float upOffset = 1.0f;
            float rightOffset = -1.0f;
            // glm::vec3 right = glm::cross(this->forward, this->up);
            // scene->projectile->velocity = this->forward * rocketSpeed;
            // scene->projectile->position = (this->eye + this->forward * (-rocketSpeed + forwardOffset) + this->up * upOffset + right * rightOffset + scene->projectile->velocity);

            // Check if charging
            if (overheating)
            {
                overheating = false;
                rocketSpeed = 1000.0f;
                chargeProgress = 0.0f;
            }

            // Projectile physics
            glm::vec3 right = glm::cross(forward, up);
            scene->projectile->velocity = forward * rocketSpeed;
            scene->projectile->position = (eye + forward * (-rocketSpeed + forwardOffset) + up * upOffset + right * rightOffset + scene->projectile->velocity);
        }
    }

    // Reload
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        reloading = true;
        scene->projectile->active = false;

        // normalize iterator between [0, 1]
        chargeProgress += deltaTime / maxChargeDuration;

        if (chargeProgress >= reloadDuration / 2.0f)
        {
            overheating = true;
        }
        else
        {
            overheating = false;
        }
    }

    // Normalize input
    if (glm::length(wishDir) > 0.001f)
        wishDir = glm::normalize(wishDir);

    wishSpeed = maxSpeed * speedMult;
    wishDir.y = 0;
    if (!airborne)
    {
        speed = glm::length(velocity);

        control = speed < stopSpeed ? stopSpeed : speed;
        drop += control * friction * deltaTime;

        newSpeed = glm::max(speed - drop, 0.0f);
        if (speed > 0)
            newSpeed /= speed;

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

    eye_prev = eye;

    // apply to player position
    eye += velocity * deltaTime;

    // Projectile physics updates
    scene->projectile->position += scene->projectile->velocity * deltaTime;
    scene->projectile->updateBounds();

    // MOVE THIS TO ANOTHER FUNCTION - pawn tracks player movement
    glm::vec3 direction = eye - glm::vec3(0, this->playerHeight, 0) - scene->pawn->position;
    // scene->pawn->position.y -= 2.0f * deltaTime;
    scene->pawn->position += direction * 1.0f * deltaTime;

    airborne = true;
    for (auto &mapGeomChild : scene->mapGeom->children)
    {
        // Check player collisions between portal or map
        if (mapGeomChild->portal->portalID > 0)
        {
            // if (AABB::intersectsCameraSinglePlane(*scene->mainCamera, *mapGeomChild))
            if (AABB::intersectsCameraSinglePlane(scene, *mapGeomChild))
            {
                std::cout << "portal collision" << std::endl;
                // Teleport to other portal
                int currPortal = mapGeomChild->portal->portalID;
                GameObject *A = scene->portals[currPortal]->portal->source;
                GameObject *B = scene->portals[currPortal]->portal->destination;

                // Teleport to the same position on the other portal (relative to the plane)
                glm::vec3 offset = eye - A->position;
                eye = B->position + offset;

                // if (mapGeomChild->portal->portalID == 1)
                // {
                // 	std::cout << mapGeomChild->objName << " id:" << mapGeomChild->id << std::endl;
                // 	glm::vec3 offset = eye - (scene->portals[0]->position + glm::vec3(0, 0.0,0)); // FIXME, z offset
                // 	eye = scene->portals[1]->position + offset;
                // 	//eye = scene->portals[1]->position + glm::vec3(0.0f, scene->portals[1]->position.y / 2.0f, -1.5f);
                // } else if (mapGeomChild->portal->portalID == 2) {
                // 	std::cout << mapGeomChild->objName << " id:" << mapGeomChild->id << std::endl;
                // 	glm::vec3 offset = eye - (scene->portals[1]->position + glm::vec3(0, 0, 0)); // FIXME, z offset
                // 	eye = scene->portals[0]->position + offset;
                // } else if (mapGeomChild->portal->portalID == 3) {
                // 	std::cout << mapGeomChild->objName << " id:" << mapGeomChild->id << std::endl;
                // 	glm::vec3 offset = eye - (scene->portals[2]->position + glm::vec3(0, playerHeight, 0)); // FIXME, z offset
                // 	eye = scene->portals[3]->position + offset;
                // } else if (mapGeomChild->portal->portalID == 4) {
                // 	std::cout << mapGeomChild->objName << " id:" << mapGeomChild->id << std::endl;
                // 	// glm::vec3 offset = eye - (scene->portals[2]->position + glm::vec3(0, -10.0f, 0)); // FIXME, z offset
                // 	// offset = glm::vec3(0);
                // 	// eye = scene->portals[2]->position + offset;
                // }
            }
        }
        // Check collision against rest of the map
        else if (AABB::intersectsConvexShape(scene, *mapGeomChild))
        {
            std::cout << mapGeomChild->objName << std::endl;
        }

        else if (AABB::intersectsCamera(scene, *scene->texture_cube))
        {
        }

        // Check projectile collisions on map
        if (AABB::intersectsObject(*scene->projectile, *mapGeomChild))
        {
            // std::cout << "projectile collision" << std::endl;
            // scene->projectile->velocity = -scene->projectile->velocity * glm::vec3(1, -1, 1);
            scene->projectile->velocity = glm::reflect(scene->projectile->velocity, mapGeomChild->planes[0].normal);
            mapGeomChild->collided = (mapGeomChild->collided % 2) + 1;

            if (scene->projectile->collided > 5)
            {
                // scene->projectile->active = false; // reset projectile
                scene->projectile->collided = 0;
            }
            else
                scene->projectile->collided++;

            // rocket jump
            // velocity.y += 10.0f;
        }

        // Check pawn collisions on map
        if (AABB::intersectsObject(*scene->pawn, *mapGeomChild))
        {
            scene->pawn->position.y = mapGeomChild->max.y + scene->pawn->scale.y / 2.0f;
        }
    }

    // AABB::intersectsCameraPlane(*scene->mainCamera, *scene->plane);
    // AABB::intersectsCameraPlane(*scene->mainCamera, *scene->angledplane);
    if (AABB::intersectsConvexShape(scene, *scene->testcube))

        // Check projectile collisions on map
        // for (auto &pawnChild : scene->pawn->children)
        // {
        // 	if (AABB::intersectsObject(*scene->projectile, *pawnChild))
        // 	{
        // 		std::cout << "projectile collision" << std::endl;
        // 		pawnChild->collided = 1;
        // 	}
        // }

        if (AABB::intersectsObject(*scene->projectile, *scene->pawn))
        {
            scene->pawn->collided = (scene->pawn->collided % 2) + 1;
        }

    if (eye.y < -400.0f)
        eye = glm::vec3(0, 50.0f, 0);
}

void Player::reloadAnimation(std::shared_ptr<SceneInitializer> &scene, float deltaTime)
{
    // normalize iterator between [0, 1]
    reloadProgress += deltaTime / reloadDuration;

    if (reloadProgress >= 1.0f && !overheating)
    {
        reloadProgress = 0.0f;
        reloading = false;
    }

    // Weapon glow
    if (!overheating)
        glUniform1f(scene->texProg->getUniform("glowIntensity"), reloadProgress); // add glow
    else
    {
        playerLightIntensity = glm::vec4(chargeProgress);
        glUniform1f(scene->texProg->getUniform("glowIntensity"), chargeProgress * reloadProgress); // add glow
    }

    // Cubic ease out f(x) = 1 - (1 - x)^3
    float easeOut = 1.0f - pow((1.0f - reloadProgress), 3.0f);

    weaponAngle = easeOut * glm::radians(360.0f);
}