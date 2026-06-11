#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "renderer/Program.h"
#include "renderer/Texture.h"
#include "world/GameObject.h"
#include "world/Camera.h"
#include "world/Player.h"
#include "world/ScreenText.h"

struct PointLightUBO
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 intensity;
};

struct LightBlockUBO
{
    PointLightUBO lights[20]; // max lights
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

    float cameraSpeed = 1000.0;
    Spline splinepath[4];

    // Testing
    bool showPortalCube = true;
    bool groundCollision = true;
    PointLightUBO debugLight;

    // Text
    TextBillboard debugText{
        .label = "debug text",
        .dynamicWorldPos = &debugLight.position,
        .color = IM_COL32(255, 50, 50, 250)};

    TextBillboard tutorialTextControls{
        .label = R"(CONTROLS LIST
                [MOUSE]         -> Look Around
                [W][A][S][D]    -> Move Around
                [Space]         -> Jump, can be held
                [Shift]         -> Sprint
                [L-Click]       -> Fire
                [R-Click]       -> Reload
                [HOLD R-Click]  -> Charge
                [R]             -> Reset position
                [ESC]           -> Exit
                
                Press [~] to unlock mouse cursor)",
        .worldPos = glm::vec3(0.0f, 40.0f, 580.0f),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextIntro{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"(Welcome!
[MOUSE]         -> Look Around)",
        .worldPos = glm::vec3(0.0f, 50.0f, 1010.0f),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextMove{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"(MOVEMENT CONTROLS
			[W][A][S][D]    -> Move Around
    [Shift]         -> Sprint)",
        .worldPos = glm::vec3(0, 50.0f, 900),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextJump{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([Space]         -> Jump)",
        .worldPos = glm::vec3(0, 50.0f, 580),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextBHop{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([HOLD Space]         -> Jump, can be held)",
        .worldPos = glm::vec3(0, 50.0f, 100),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextBHopRamp{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([HOLD Space]         -> Jump, can be held on ramps)",
        .worldPos = glm::vec3(0, 50.0f, -425),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextSurfRamp{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([HOLD Space]         -> Surf, can be held on surfing ramps)",
        .worldPos = glm::vec3(950, -10.0f, -3100),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextReset{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([R]             -> Reset position)",
        .worldPos = glm::vec3(0, 50.0f, -950),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextResetCheckpoint1{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([1]             -> Reset position)",
        .worldPos = glm::vec3(700.0f, -10.0f, -3250.0f),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextResetCheckpoint2{
        .minFadeDistance = 200.0f,
        .maxFadeDistance = 300.0f,
        .label = R"([2]             -> Reset position)",
        .worldPos = glm::vec3(0, 50.0f, -950),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextResetFall{
        .minFadeDistance = 400.0f,
        .maxFadeDistance = 500.0f,
        .label = R"([R]             -> Reset position)",
        .worldPos = glm::vec3(0, -460.0f, -1000),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextBranchingPath{
        .minFadeDistance = 500.0f,
        .maxFadeDistance = 1000.0f,
        .label = R"(LEFT - Easier   RIGHT - Harder
Remember to hold [SHIFT] to go faster
    
Checkpoint unlocked, press [2])",
        .worldPos = glm::vec3(770, -400.0f, -600),
        .color = IM_COL32(255, 255, 255, 250)};

    TextBillboard tutorialTextCongratulations{
        .minFadeDistance = 500.0f,
        .maxFadeDistance = 800.0f,
        .label = R"(Congratulations on beating the tutorial!)",
        .worldPos = glm::vec3(770, -490.0f, -4350),
        .color = IM_COL32(255, 255, 255, 250)};

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
    std::shared_ptr<GameObject> portalcubeMirror;
    std::shared_ptr<GameObject> testcube;
    std::shared_ptr<GameObject> arrow;
    std::shared_ptr<GameObject> tool;
    std::shared_ptr<GameObject> projectile;
    std::shared_ptr<GameObject> pawn;
    std::shared_ptr<GameObject> shape_performance_test;
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