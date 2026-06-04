#pragma once

#include "../physics/CollisionType.h"

class SceneRender;
class SceneInitializer;

class Player
{
private:
public:
    void playerMovement(GLFWwindow *window, std::shared_ptr<SceneInitializer> &scene, float maxSpeed, float deltaTime);
    void reloadAnimation(std::shared_ptr<SceneInitializer> &scene, float deltaTime);
    void resolveCollision(std::shared_ptr<SceneInitializer> &scene, CollisionPlaneResult collisionInfo);

    // position data
    float playerHeight = 46.0f;
    float playerRadius = 4.0f;
    glm::vec3 spawnLocation = glm::vec3(0.0f, playerHeight + 50.0f, 1000.0f);
    glm::vec3 checkPoint1 = glm::vec3(700.0f, playerHeight + 50.0f, -3250.0f);
    glm::vec3 checkPoint2 = glm::vec3(770.0f, -400.0f + playerHeight + 50.0f, -600.0f);

    // physics data
    bool airborne = false;
    float gravity = 800.0f;
    glm::vec3 wishDir = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    // tool data
    bool overheating = false;
    float chargeProgress = 0.0f;
    float maxChargeDuration = 2.0f;

    // animation data
    bool reloading = false;
    float reloadProgress = 0.0f;
    float reloadDuration = 1.0f;
    float weaponAngle = 0.0f;
    glm::vec4 playerLightIntensity;
};