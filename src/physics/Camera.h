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

    glm::vec3 view          = glm::vec3(0, 0, 1); // FIXME, change names of these variables
    glm::vec3 g_eye         = glm::vec3(0, 1, 0);
    glm::vec3 g_eye_prev    = glm::vec3(0, 1, 0);
    glm::vec3 g_strafe      = glm::vec3(1, 0, 0);
    glm::vec3 g_up          = glm::vec3(0, 1, 0);
    glm::vec3 g_forward     = glm::vec3(0, 0, 1);
    glm::vec3 g_lookAt      = glm::vec3(0, 1, -4);
};

#endif