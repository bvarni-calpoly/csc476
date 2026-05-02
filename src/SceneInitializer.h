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
    void loadGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj);
    void loadHierGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj);
    void loadMapGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj);
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
    std::shared_ptr<Camera> tempCamera = std::make_shared<Camera>();

    float cameraSpeed = 10.0;
    Spline splinepath[4];

    // scene / level
    std::shared_ptr<GameObject> player;
    std::shared_ptr<GameObject> mapGeom;
    std::shared_ptr<GameObject> mapGeomNoHier;
    std::shared_ptr<GameObject> skybox;
    std::shared_ptr<GameObject> portalEntranceDoor;
    std::shared_ptr<GameObject> portalExitDoor;
    glm::vec3 portalEntrancePos = glm::vec3(5.0f, 0.0f, 0.0f);
    glm::vec3 portalExitPos = glm::vec3(10.0f, 0.0f, 0.0f);

    std::shared_ptr<GameObject> cube;
    std::shared_ptr<GameObject> arrow;
    glm::vec3 gMin;

    int objectCount = 0;
    int objectCollisionCount = 0;

    // Create the matrix stacks
    std::shared_ptr<MatrixStack> Projection;
    std::shared_ptr<MatrixStack> ProjectionPortal;
    std::shared_ptr<MatrixStack> Model;
    std::shared_ptr<MatrixStack> ModelPortalSource;
    std::shared_ptr<MatrixStack> ModelPortalDestination;
};