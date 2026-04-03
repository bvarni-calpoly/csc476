#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
private:
    /* data */
public:
    Camera(/* args */);
    ~Camera();

    void cameraMovement(GLFWwindow *window, float cameraSpeed, float deltaTime);
	void playerMovement(GLFWwindow *window, float cameraSpeed, float deltaTime);
   	void SetView(std::shared_ptr<Program> shader);
    void updateUsingCameraPath(float frametime, Spline *splinepath);

    glm::vec3 view          = glm::vec3(0, 0, 1);
    glm::vec3 eye           = glm::vec3(0, 1, 0);
    glm::vec3 eye_prev      = glm::vec3(0, 1, 0);
    glm::vec3 strafe        = glm::vec3(1, 0, 0);
    glm::vec3 up            = glm::vec3(0, 1, 0);
    glm::vec3 forward       = glm::vec3(0, 0, 1);
    glm::vec3 lookAtTarget  = glm::vec3(0, 1, -4);
};

#endif