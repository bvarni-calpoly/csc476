#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "../core/WindowManager.h"
#include "../renderer/Program.h"
#include "../renderer/MatrixStack.h"
#include "../math/Spline.h"
#include <memory>

class SceneInitializer;

class Camera
{
private:
    /* data */
public:
    Camera(/* args */);
    ~Camera();

    void cameraMovement(GLFWwindow *window, float cameraSpeed, float deltaTime); //fixme make deltatime a variable
	void playerMovement(GLFWwindow *window, std::shared_ptr<SceneInitializer> scene, float maxSpeed, float deltaTime);
   	void SetView(std::shared_ptr<Program> shader);
   	void SetPortalView(std::shared_ptr<Program> shader, std::shared_ptr<Camera> mainCamera, std::shared_ptr<MatrixStack> sourcePortal, std::shared_ptr<MatrixStack> destinationPortal);
    void SetRecursivePortalView(std::shared_ptr<Program> shader, std::shared_ptr<Camera> mainCamera, std::shared_ptr<MatrixStack> sourcePortal, std::shared_ptr<MatrixStack> destinationPortal);
    void updateUsingCameraPath(float frametime, Spline *splinepath);

    glm::vec3 view          = glm::vec3(0, 0, 1);
    glm::vec3 eye           = glm::vec3(0, 50, 0);
    glm::vec3 eye_prev      = glm::vec3(0, 1, 0);
    glm::vec3 strafe        = glm::vec3(1, 0, 0);
    glm::vec3 up            = glm::vec3(0, 1, 0);
    glm::vec3 forward       = glm::vec3(0, 0, 1);
    glm::vec3 lookAtTarget  = glm::vec3(0, 1, 0);

    // move into player class
	bool airborne = false;
	float gravity = 400;
    float playerHeight = 16.0f;
	glm::vec3 velocity = glm::vec3(0);
};

#endif