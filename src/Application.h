#pragma once

// fixme dependencies
#include "SceneInitializer.h"
#include "SceneRender.h"
#include "core/WindowManager.h"
#include "core/Callbacks.h"
#include "world/GameObject.h"
#include "world/Camera.h"
#include "math/Spline.h"
#include "renderer/Texture.h"

// FIXME CREATE FORWARD DEPENDENCIES
class WindowManager;
class Program;
class Shape;
class MatrixStack;

class Application : public EventCallbacks
{
private:
    /* data */
public:
    Application(float dt);
    ~Application();
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mouseCallback(GLFWwindow *window, int button, int action, int mods);
    void scrollCallback(GLFWwindow *window, double deltaX, double deltaY);
    void setCursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    void resizeCallback(GLFWwindow *window, int width, int height);

    void init(const std::string &resourceDirectory);
    void render(float frametime);

    // void drawHierModel(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::vector<std::shared_ptr<Shape>> Shape, glm::vec3 min, glm::vec3 max, int material = 0, glm::vec3 trans = glm::vec3(0.0), float rotateDeg = 0, glm::vec3 rotate = glm::vec3(0.0), glm::vec3 scale = glm::vec3(1.0));

    std::shared_ptr<SceneInitializer> scene;
    std::shared_ptr<SceneRender> sceneRender;
    std::shared_ptr<Callbacks> callbacks;

    float deltaTime;
    int windowWidth = 1600, windowHeight = 1000; // MOVE THIS TO SCENE INIT

    WindowManager *windowManager = nullptr;

    // animation data
    float timer = 1.0f;
    float g_Spin = 3.14 / 180 * 2; // 2 deg
    float sTheta = 0;
    float eTheta = 0;
    float hTheta = 0;
};