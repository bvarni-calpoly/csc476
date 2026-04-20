#pragma once

#include <string>
#include <memory>
#include "renderer/Program.h"
#include "renderer/Texture.h"
#include "world/GameObject.h"
#include "world/Camera.h"

class SceneInitializer
{
private:
    /* data */
public:
    SceneInitializer(/* args */);
    ~SceneInitializer();

    void init(const std::string &resourceDirectory);
    void initGeom(const std::string &resourceDirectory);
    void initGround();

    // global data for ground plane - direct load constant defined CPU data to GPU (not obj)
    GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
    int g_GiboLen;
    // ground VAO
    GLuint GroundVertexArrayID;
    
    // Our shader program - use this one for Blinn-Phong has diffuse
    std::shared_ptr<Program> prog;
    std::shared_ptr<Program> texProg; // Our shader program for textures
    std::shared_ptr<Program> debugShader; // shader program for debug information and collision visualization

    // the image to use as a texture
    std::shared_ptr<Texture> texture0;
    std::shared_ptr<Texture> texture1;

    // Camera
    std::shared_ptr<Camera> mainCamera = std::make_shared<Camera>();
    std::shared_ptr<Camera> portalCamera = std::make_shared<Camera>();
    float cameraSpeed = 5.0;
    Spline splinepath[4];

    // scene / level
    std::shared_ptr<GameObject> scene;
    std::shared_ptr<GameObject> skybox;
    std::shared_ptr<GameObject> portalDoor;

    std::shared_ptr<GameObject> cube;
    std::shared_ptr<GameObject> arrow;
    glm::vec3 gMin;

    int objectCount = 0;
    int objectCollisionCount = 0;
};