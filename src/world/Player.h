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

    bool airborne = false;
    float gravity = 400.0f;
    float playerHeight = 40.0f;
    float playerRadius = 4.0f;
    glm::vec3 wishDir = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0);

    glm::vec4 playerLightIntensity;

    // tool data
    bool overheating = false;
    float chargeProgress = 0.0f;
    float maxChargeDuration = 2.0f;

    // animation data
    bool reloading = false;
    float reloadProgress = 0.0f;
    float reloadDuration = 1.0f;
    float weaponAngle = 0.0f;
};