#pragma once

// fixme dependencies
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
    void initGeom(const std::string &resourceDirectory);
    void render(float frametime);

    void initGround();
    void drawHierMap(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::vector<std::shared_ptr<Shape>> shape);
    void drawGround(std::shared_ptr<Program> curS);
    void drawSkybox(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<Shape> shape);
    void SetMaterial(std::shared_ptr<Program> curS, int i);
    void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS);
    void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> M);
    // void drawHierModel(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::vector<std::shared_ptr<Shape>> Shape, glm::vec3 min, glm::vec3 max, int material = 0, glm::vec3 trans = glm::vec3(0.0), float rotateDeg = 0, glm::vec3 rotate = glm::vec3(0.0), glm::vec3 scale = glm::vec3(1.0));

    float deltaTime;
    int windowWidth = 1920, windowHeight = 1080;

    WindowManager *windowManager = nullptr;

    // Our shader program - use this one for Blinn-Phong has diffuse
    std::shared_ptr<Program> prog;
    std::shared_ptr<Program> texProg; // Our shader program for textures
    std::shared_ptr<Program> debugShader; // shader program for debug information and collision visualization

    // Camera
    std::shared_ptr<Camera> mainCamera = std::make_shared<Camera>();
    std::shared_ptr<Callbacks> callbacks;
    float cameraSpeed = 5.0;

    // scene / level
    std::shared_ptr<GameObject> scene;
    std::shared_ptr<GameObject> skybox;

    std::shared_ptr<GameObject> cube;
    std::shared_ptr<GameObject> arrow;
    glm::vec3 gMin;

    int objectCount = 0;
    int objectCollisionCount = 0;

    // global data for ground plane - direct load constant defined CPU data to GPU (not obj)
    GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
    int g_GiboLen;
    // ground VAO
    GLuint GroundVertexArrayID;

    // the image to use as a texture
    std::shared_ptr<Texture> texture0;
    std::shared_ptr<Texture> texture1;

    // animation data
    float timer = 1.0f;
    float g_Spin = 3.14 / 180 * 2; // 2 deg
    float sTheta = 0;
    float eTheta = 0;
    float hTheta = 0;

    // player
    Spline splinepath[4];
};