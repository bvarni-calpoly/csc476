#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "renderer/Program.h"
#include "renderer/Texture.h"
#include "world/GameObject.h"
#include "world/Camera.h"
#include "world/Player.h"

struct PointLightUBO
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 intensity;
};

struct LightBlockUBO
{
    PointLightUBO lights[10];
    glm::ivec4 numActiveLights;
};

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

    // Uniform Buffer Object for scene lights
    unsigned int uboLightBlock;

    // Framebuffer
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int quadVAO, quadVBO;
    unsigned int fbo;

    // Bloom framebuffers
    unsigned int hdrFBO;
    unsigned int colorBuffers[2];

    // Gaussian blur
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffers[2];

    int windowWidth = 1600, windowHeight = 1000;

    // Our shader program - use this one for Blinn-Phong has diffuse
    std::shared_ptr<Program> prog;
    std::shared_ptr<Program> texProg;         // Our shader program for textures
    std::shared_ptr<Program> screenShader;    // Our shader for post processing effects with fbo
    std::shared_ptr<Program> blurShader;      // Our shader program for gaussian blur
    std::shared_ptr<Program> blendShader;     // Our shader program for bloom
    std::shared_ptr<Program> debugShader;     // shader program for debug information and collision visualization
    std::shared_ptr<Program> debugNormShader; // shader program for normal vector visualization

    // the image to use as a texture
    std::shared_ptr<Texture> texture0;
    std::shared_ptr<Texture> texture1;
    std::shared_ptr<Texture> textureTile;
    std::shared_ptr<Texture> textureBlue;
    std::shared_ptr<Texture> texturePurple;
    std::shared_ptr<Texture> textureWhiteTile;
    std::shared_ptr<Texture> textureBlackTile;
    std::shared_ptr<Texture> textureBlackStripeTile;
    std::shared_ptr<Texture> textureBlackBrick;
    std::shared_ptr<Texture> textureBlackWhiteTile;

    // Portal map
    std::unordered_map<int, GameObject *> portals; // id, obj - raw pointer because scene graph owns the unique_ptr

    // Cameras
    std::shared_ptr<Player> playerCamera = std::make_shared<Player>();
    std::shared_ptr<Camera> mainCamera = std::make_shared<Camera>();
    std::shared_ptr<Camera> portalCamera = std::make_shared<Camera>();
    std::shared_ptr<Camera> tempCamera = std::make_shared<Camera>();

    float cameraSpeed = 500.0;
    Spline splinepath[4];

    // Testing
    bool showPortalCube = false;
    bool groundCollision = false;
    PointLightUBO testLight;

    // scene / level
    std::shared_ptr<GameObject> player;
    std::shared_ptr<GameObject> mapGeom;
    std::shared_ptr<GameObject> mapGeomNoHier;
    std::shared_ptr<GameObject> skybox;
    std::shared_ptr<GameObject> cube;
    std::shared_ptr<GameObject> texture_cube;
    std::shared_ptr<GameObject> plane;
    std::shared_ptr<GameObject> angledplane;
    std::shared_ptr<GameObject> portalcube;
    std::shared_ptr<GameObject> testcube;
    std::shared_ptr<GameObject> arrow;
    std::shared_ptr<GameObject> tool;
    std::shared_ptr<GameObject> projectile;
    std::shared_ptr<GameObject> pawn;
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