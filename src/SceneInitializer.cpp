#include <iostream>
#include "SceneInitializer.h"
#include "renderer/Program.h"
#include "math/Spline.h"
#include "core/GLSL.h"

using namespace std;
using namespace glm;

SceneInitializer::SceneInitializer(/* args */) {}

SceneInitializer::~SceneInitializer() {}

void SceneInitializer::init(const std::string &resourceDirectory)
{
    GLSL::checkVersion();

    // might not work on WSL?
    // glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set background color.
    glClearColor(.72f, .84f, 1.06f, 1.0f);
    // Enable z-buffer test.
    glEnable(GL_DEPTH_TEST);

    // Initialize the GLSL program that we will use for local shading
    prog = make_shared<Program>();
    prog->setVerbose(true);
    prog->setShaderNames(resourceDirectory + "/shaders/mat_vert.glsl", resourceDirectory + "/shaders/mat_frag.glsl");
    prog->init();
    prog->addUniform("P");
    prog->addUniform("V");
    prog->addUniform("M");
    prog->addUniform("MatAmb");
    prog->addUniform("MatDif");
    prog->addUniform("MatSpec");
    prog->addUniform("MatShine");
    prog->addUniform("lightPos");
    prog->addAttribute("vertPos");
    prog->addAttribute("vertNor");
    prog->addAttribute("vertTex"); // silence error

    // Initialize the GLSL program that we will use for texture mapping
    texProg = make_shared<Program>();
    texProg->setVerbose(true);
    texProg->setShaderNames(resourceDirectory + "/shaders/tex_vert.glsl", resourceDirectory + "/shaders/tex_frag.glsl");
    texProg->init();
    texProg->addUniform("P");
    texProg->addUniform("V");
    texProg->addUniform("M");
    texProg->addUniform("flip");
    texProg->addUniform("Texture0");
    texProg->addUniform("MatShine");
    texProg->addUniform("lightPos");
    texProg->addAttribute("vertPos");
    texProg->addAttribute("vertNor");
    texProg->addAttribute("vertTex");

    // Initialize the GLSL program for rendering collision bounds
    debugShader = make_shared<Program>();
    debugShader->setVerbose(true);
    debugShader->setShaderNames(resourceDirectory + "/shaders/debug.vert", resourceDirectory + "/shaders/debug.frag");
    debugShader->init();
    debugShader->addUniform("P");
    debugShader->addUniform("V");
    debugShader->addUniform("M");
    debugShader->addUniform("debugColor");
    debugShader->addUniform("camPos");
    debugShader->addUniform("boxMin");
    debugShader->addUniform("boxMax");
    debugShader->addUniform("collided");
    debugShader->addAttribute("vertPos");
    debugShader->addAttribute("vertNor"); // silence error
    debugShader->addAttribute("vertTex"); // silence error

    // read in a load the texture
    texture0 = make_shared<Texture>();
    texture0->setFilename(resourceDirectory + "/image.jpg");
    texture0->init();
    texture0->setUnit(0);
    texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    texture1 = make_shared<Texture>();
    texture1->setFilename(resourceDirectory + "/image.jpg");
    texture1->init();
    texture1->setUnit(1);
    texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    // init splines up and down
    splinepath[0] = Spline(glm::vec3(0, 2, 0), glm::vec3(-5, 2, 0), glm::vec3(5, 2, 0), glm::vec3(0, 4, -2), 2);
    splinepath[1] = Spline(glm::vec3(0, 4, -2), glm::vec3(8, 6, 0), glm::vec3(0, 4, 2), 2);
    splinepath[2] = Spline(glm::vec3(0, 4, 2), glm::vec3(-8, 6, 0), glm::vec3(0, 4, -2), 2);
    splinepath[3] = Spline(glm::vec3(0, 4, -2), glm::vec3(1, 2, 0), glm::vec3(-1, 2, 0), glm::vec3(0, 2, 0), 2);
}

void SceneInitializer::loadGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj)
{
    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    // load in the mesh and make the shape(s)
    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + fileName).c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        obj = make_shared<GameObject>();
        obj->shape = make_shared<Shape>();
        obj->shape->createShape(TOshapes[0]);
        obj->shape->measure();
        obj->shape->init();

        obj->localMin = obj->shape->min;
        obj->localMax = obj->shape->max;

        obj->updateBounds(); // FIXME
    }
}

void SceneInitializer::initGeom(const std::string &resourceDirectory)
{
    // EXAMPLE set up to read one shape from one obj file - convert to read several
    //  Initialize mesh
    //  Load geometry
    //  Some obj files contain material information.We'll ignore them for this assignment.
    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    // load in the mesh and make the shape(s)
    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/objects/cube.obj").c_str());
    /*
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        cube = make_shared<GameObject>();
        cube->shape = make_shared<Shape>();
        cube->shape->createShape(TOshapes[0]);
        cube->shape->measure();
        cube->shape->init();
    }
    */

    // Initialize map mesh (hier)
    vector<tinyobj::shape_t> TOshapesScene;
    vector<tinyobj::material_t> objMaterialsScene;
    // load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapesScene, objMaterialsScene, errStr, (resourceDirectory + "/scene/blender_test.obj").c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        mapGeom = make_shared<GameObject>();

        for (int i = 0; i < TOshapesScene.size(); i++)
        {
            // Create temporary empty part
            auto part = make_unique<GameObject>();

            // Initialize scene part
            part->shape = make_shared<Shape>();
            part->shape->createShape(TOshapesScene[i]);
            part->shape->measure();
            part->shape->init();

            part->localMin = part->shape->min;
            part->localMax = part->shape->max;

            part->updateBounds();

            // Add part to scene vector
            // scene->min = min(scene->min, part->min);
            // scene->max = max(scene->max, part->max);

            mapGeom->addChild(move(part));
        }
    }

    loadGeom(resourceDirectory, "/objects/cube.obj", skybox);
    loadGeom(resourceDirectory, "/objects/cube.obj", portalEntranceDoor);
    loadGeom(resourceDirectory, "/objects/cube.obj", portalExitDoor);
    loadGeom(resourceDirectory, "/scene/blender_test.obj", mapGeomNoHier);

    // Initialize arrow mesh
    vector<tinyobj::shape_t> TOshapesArrow;
    vector<tinyobj::material_t> objMaterialsArrow;
    // load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapesArrow, objMaterialsArrow, errStr, (resourceDirectory + "/objects/wedge.obj").c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        arrow = make_shared<GameObject>();
        arrow->shape = make_shared<Shape>();
        arrow->shape->createShape(TOshapesArrow[0]);
        arrow->shape->measure();
        arrow->shape->init();

        arrow->localMin = arrow->shape->min;
        arrow->localMax = arrow->shape->max;
    }

    // Initialize cube mesh
    vector<tinyobj::shape_t> TOshapesCube;
    vector<tinyobj::material_t> objMaterialsCube;
    // load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapesCube, objMaterialsCube, errStr, (resourceDirectory + "/objects/cube.obj").c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        cube = make_shared<GameObject>();
        cube->shape = make_shared<Shape>();
        cube->shape->createShape(TOshapesCube[0]);
        cube->shape->measure();
        cube->shape->init();

        cube->localMin = cube->shape->min;
        cube->localMax = cube->shape->max;
    }

    // code to load in the ground plane (CPU defined data passed to GPU)
    // initGround();

    // create gameobjects for arrow
    for (int i = 0; i < 10; i++)
    {
        // create children nodes for arrow
        auto arrowChild = make_unique<GameObject>(
            arrow->shape,
            vec3(rand() % 10 + 2, 0.0f, rand() % 10 + 2),
            0.0f,
            vec3(0.0f, radians((float)(rand() % 360)), 0.0f),
            vec3(1.0f),
            arrow->localMin,
            arrow->localMax);
        arrowChild->velocity = vec3(rand() % 10, 0, rand() % 10);

        arrowChild->updateBounds();
        arrow->addChild(std::move(arrowChild));
        objectCount += 1;
    }

    // create gameobjects for cube walls
    for (int i = 0; i < 4; i++)
    {
        // create children nodes for arrow
        auto cubeChild = make_unique<GameObject>(
            cube->shape,
            vec3(0.0f),
            0.0f,
            vec3(0.0f),
            vec3(1.0f),
            cube->localMin,
            cube->localMax);
        cubeChild->velocity = vec3(0);

        cubeChild->updateBounds();
        cube->addChild(std::move(cubeChild));
    }
    float wallLength = 20.0f;
    cube->children[0]->scale = vec3(1.0f, 1.0f, wallLength);
    cube->children[0]->position = vec3(0.0f, 0.0f, wallLength / 2);

    cube->children[1]->scale = vec3(wallLength, 1.0f, 1.0f);
    cube->children[1]->position = vec3(wallLength / 2, 0.0f, 0.0f);

    cube->children[2]->scale = vec3(1.0f, 1.0f, wallLength);
    cube->children[2]->position = vec3(wallLength, 0.0f, wallLength / 2);

    cube->children[3]->scale = vec3(wallLength, 1.0f, 1.0f);
    cube->children[3]->position = vec3(wallLength / 2, 0.0f, wallLength);
}

// directly pass quad for the ground to the GPU
void SceneInitializer::initGround()
{
    float g_groundSize = 20;
    float g_groundY = -0.25;

    // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
        -g_groundSize, g_groundY, -g_groundSize,
        -g_groundSize, g_groundY, g_groundSize,
        g_groundSize, g_groundY, g_groundSize,
        g_groundSize, g_groundY, -g_groundSize};

    float GrndNorm[] = {
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0};

    static GLfloat GrndTex[] = {
        0, 0, // back
        0, 1,
        1, 1,
        1, 0};

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};

    // generate the ground VAO
    glGenVertexArrays(1, &GroundVertexArrayID);
    glBindVertexArray(GroundVertexArrayID);

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}