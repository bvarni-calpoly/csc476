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
#include "../physics/CollisionSolver.h"
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

    wishDir = glm::vec3(0.0f);
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
            velocity.y = 300;
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
    bool isLeftMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (isLeftMousePressed && !wasLeftMousePressed)
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

            // Rocket jump
            velocity += -750.0f * scene->mainCamera->forward;
        }

        pickupObj = !pickupObj;
    }

    wasLeftMousePressed = isLeftMousePressed;
    if (!pickupObj)
    {
        float objRaycast = CollisionSolver::solveRaycastVsAABB(eye, forward, *scene->portalcube);
        if (objRaycast > 0.0f)
        {
            objRaycastHit = true;
            pickupDist = objRaycast;
        }
        else
        {
            objRaycastHit = false;
            pickupObj = false; // prevent picking up object state without looking at it
        }
    }

    if (pickupObj)
    {
        if (pickupDist < 50.0f)
            pickupDist = 50.0f;

        scene->portalcube->position = eye + (forward * pickupDist);
        scene->portalcube->updateBounds();
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

    // Reset position
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        velocity = glm::vec3(0.0f);
        scene->mainCamera->eye = spawnLocation;
    }

    // Reset position
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        velocity = glm::vec3(0.0f);
        scene->mainCamera->eye = checkPoint1;
    }

    // Reset position
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        velocity = glm::vec3(0.0f);
        scene->mainCamera->eye = checkPoint2;
    }

    // Normalize input
    if (glm::length(wishDir) > 0.001f)
        wishDir = glm::normalize(wishDir);

    wishSpeed = maxSpeed * speedMult;
    wishDir.y = 0;

    // Ground friction
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
    // --- TEST COLLISIONS  ---
    for (auto &mapGeomChild : scene->mapGeom->children)
    {
        // Check player collisions between portal or map
        if (mapGeomChild->portal->portalID > 0)
        {
            // if (CollisionSolver::solvePlayerVsPlane(*scene->mainCamera, *mapGeomChild))
            if (CollisionSolver::testPlayerVsPlane(scene, *mapGeomChild))
            {
                std::cout << "portal collision" << std::endl;

                // Teleport to other portal
                int currPortal = mapGeomChild->portal->portalID;
                GameObject *A = scene->portals[currPortal]->portal->source;
                GameObject *B = scene->portals[currPortal]->portal->destination;

                // Teleport to the same position on the other portal (relative to the plane)
                glm::vec3 offset = eye - A->position;
                eye = B->position + offset;
            }
            // portal cube teleporation
            if (CollisionSolver::testObjVsPlane(*scene->portalcube, *mapGeomChild))
            {
                std::cout << "portalcube collision with portal" << std::endl;
                // Teleport to other portal
                int currPortal = mapGeomChild->portal->portalID;
                GameObject *A = scene->portals[currPortal]->portal->source;
                GameObject *B = scene->portals[currPortal]->portal->destination;

                // Teleport to the same position on the other portal (relative to the plane)
                glm::vec3 offset = scene->portalcubeMirror->position - A->position;
                scene->portalcubeMirror->position = B->position + offset;
            }
        }
        // Check collision against rest of the map
        else
        {
            // Broad Phase (AABB)
            if (CollisionSolver::testPlayerVsAABB(scene, *mapGeomChild))
            {
                // Narrow Phase (Convex Plane Shape)
                CollisionPlaneResult collisionInfo = CollisionSolver::solvePlayerVsConvex(scene, *mapGeomChild);

                if (collisionInfo.collided)
                {
                    // scene->playerCamera->wallWalk(scene, collisionInfo);
                    scene->playerCamera->resolveCollision(scene, collisionInfo);
                }
            }
        }

        // Global test ground
        if (scene->groundCollision && CollisionSolver::testPlayerVsAABB(scene, *scene->texture_cube))
        {
            Camera &cam = *(scene->mainCamera);
            Player &player = *(scene->playerCamera);

            cam.eye.y = scene->texture_cube->max.y + player.playerHeight + player.playerRadius;

            airborne = false;
        }

        // Player collision against single shape
        // if (CollisionSolver::testPlayerVsAABB(scene, *scene->shape_performance_test))
        // {
        //     CollisionPlaneResult collisionInfoShapeTest = CollisionSolver::solvePlayerVsConvex(scene, *scene->shape_performance_test);

        //     if (collisionInfoShapeTest.collided)
        //     {
        //         scene->playerCamera->resolveCollision(scene, collisionInfoShapeTest);
        //     }
        // }

        // Check projectile collisions on map
        // Broad Phase (AABB)
        if (CollisionSolver::testAABBvsAABB(*scene->projectile, *mapGeomChild))
        {
            // Narrow Phase (Convex Plane Shape)
            CollisionPlaneResult collisionInfo = CollisionSolver::solvePointVsConvex(scene->projectile->position, *mapGeomChild);

            // if (collisionInfo.collided)
            scene->projectile->velocity = glm::reflect(scene->projectile->velocity, collisionInfo.normal);
        }

        // Check portal cubes collisions on map
        // Broad Phase (AABB)
        if (CollisionSolver::testAABBvsAABB(*scene->portalcube, *mapGeomChild))
        {
            // Narrow Phase (Convex Plane Shape)
            CollisionPlaneResult collisionInfo = CollisionSolver::solvePointVsConvex(scene->portalcube->position, *mapGeomChild);

            // if (collisionInfo.collided)
            const glm::vec3 &pushDir = collisionInfo.normal;
            float closestPlaneDistance = collisionInfo.planeDistance;

            // offset position to be on correct side of the closest plane that was passed
            // float epsilon = 0.01;
            // glm::vec3 correction = pushDir * (glm::abs(closestPlaneDistance) + epsilon);
            // scene->portalcube->position += correction * 10.0f;

            // only on floors
            // if (pushDir.y < 0.7f)
            // {
            float height = scene->portalcube->max.y - scene->portalcube->min.y;
            scene->portalcube->position.y = mapGeomChild->max.y + height / 2.0f;
            // }
        }

        // Check pawn collisions on map
        if (CollisionSolver::testAABBvsAABB(*scene->pawn, *mapGeomChild))
        {
            scene->pawn->position.y = mapGeomChild->max.y + scene->pawn->scale.y / 2.0f;
        }
    }

    if (CollisionSolver::testAABBvsAABB(*scene->projectile, *scene->pawn))
    {
        scene->pawn->collided = (scene->pawn->collided % 2) + 1;
    }

    if (eye.y < -1000.0f)
        eye = spawnLocation;
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

void Player::resolveCollision(std::shared_ptr<SceneInitializer> &scene, CollisionPlaneResult collisionInfo)
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);
    const glm::vec3 &pushDir = collisionInfo.normal;
    float closestPlaneDistance = collisionInfo.planeDistance;

    // offset position to be on correct side of the closest plane that was passed
    float epsilon = 0.01;
    glm::vec3 correction = pushDir * (glm::abs(closestPlaneDistance) + epsilon);
    // glm::vec3 correction = pushDir * (glm::abs(closestPlaneDistance) + playerRadius);
    cam.eye += correction;

    // push player out in direction of normal
    float velocityAlongNormal = glm::dot(player.velocity, pushDir);
    if (velocityAlongNormal < 0.0f)
    {
        player.velocity -= pushDir * velocityAlongNormal;
    }

    // Check if collision is a floor (y close to 1, angle > 45 degrees)
    if (pushDir.y > 0.7) // sin(45) = 0.707
    {
        // remove sliding
        player.velocity.y = 0.0f;
        player.airborne = false;
    }
    // Check if collision is a ceiling (y close to -1, angle < -45 degrees)
    else if (pushDir.y < -0.7)
    {
        player.velocity.y = 0.0f;
    }
    // Check if collision is a wall (y close to 0, angle between -45 to 45 degrees)
    else
    {
        // wall
    }
}

// void Player::wallWalk(std::shared_ptr<SceneInitializer> &scene, CollisionPlaneResult collisionInfo)
// {
//     Camera &cam = *(scene->mainCamera);
//     Player &player = *(scene->playerCamera);
//     const glm::vec3 &floorUp = collisionInfo.normal;
//     float closestPlaneDistance = collisionInfo.planeDistance;

//     cam.up = glm::normalize(floorUp);
//     cam.strafe = glm::cross(cam.forward, cam.up);
//     cam.forward = glm::cross(cam.up, cam.strafe);
// }