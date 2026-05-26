#pragma once

class SceneRender;
class SceneInitializer;

class Player
{
private:
public:
    void playerMovement(GLFWwindow *window, std::shared_ptr<SceneInitializer> &scene, float maxSpeed, float deltaTime);
    void reloadAnimation(std::shared_ptr<SceneInitializer> &scene, float deltaTime);

    bool airborne = false;
    float gravity = 400.0f;
    float playerHeight = 40.0f;
    glm::vec3 velocity = glm::vec3(0);

    // tool data
    bool isCharging = false;
    float chargeProgress = 0.0f;
    float maxChargeTime = 2.0f;
    bool overheating = false;

    // animation data
    bool reloading = false;
    float reloadProgress = 0.0f;
    float reloadDuration = 1.0f;
    float weaponAngle = 0.0f;
};