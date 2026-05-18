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
    texProg->addUniform("Texture1");
    texProg->addUniform("TextureBlue");
    texProg->addUniform("TexturePurple");
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

    // Initialize the GLSL program for rendering normal vectors
    debugNormShader = make_shared<Program>();
    debugNormShader->setVerbose(true);
    debugNormShader->setShaderNames(resourceDirectory + "/shaders/debug_normal.vert", resourceDirectory + "/shaders/debug_normal.frag", resourceDirectory + "/shaders/debug_normal.geom");
    debugNormShader->init();
    debugNormShader->addUniform("V");
    debugNormShader->addUniform("M");
    debugNormShader->addUniform("P");
    debugNormShader->addAttribute("vertPos");
    debugNormShader->addAttribute("vertNor");
    debugNormShader->addAttribute("vertTex"); // silence error

    // read in a load the texture
    texture0 = make_shared<Texture>();
    texture0->setFilename(resourceDirectory + "/image.jpg");
    texture0->init();
    texture0->setUnit(0);
    texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    texture1 = make_shared<Texture>();
    texture1->setFilename(resourceDirectory + "/scene/textures/test_texture.jpg");
    texture1->init();
    texture1->setUnit(1);
    texture1->setWrapModes(GL_REPEAT, GL_REPEAT);

    textureBlue = make_shared<Texture>();
    textureBlue->setFilename(resourceDirectory + "/scene/textures/blue_test_texture.jpg");
    textureBlue->init();
    textureBlue->setUnit(2);
    textureBlue->setWrapModes(GL_REPEAT, GL_REPEAT);

    texturePurple = make_shared<Texture>();
    texturePurple->setFilename(resourceDirectory + "/scene/textures/purple_test_texture.jpg");
    texturePurple->init();
    texturePurple->setUnit(3);
    texturePurple->setWrapModes(GL_REPEAT, GL_REPEAT);

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

        obj->updateBounds();

        // Read normals from obj file
        const auto& meshNormals = TOshapes[0].mesh.normals;
        
        if (!meshNormals.empty())
        {
            for (int i = 0; i < meshNormals.size(); i+=3)
            {
                // int normalIdx = TOshapes[0].mesh.indices;
                glm::vec3 objNormal = glm::vec3(meshNormals[i + 0], meshNormals[i + 1], meshNormals[i + 2]);
                obj->normals.push_back(glm::normalize(objNormal));
            }
        }
        else
        {
            std::cout << "No normals in obj file" << std::endl;
        }
    }
}

void SceneInitializer::loadHierGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj)
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

        for(int i = 0; i < TOshapes.size(); i++)
        {
            auto part = make_unique<GameObject>();

            part->shape = make_shared<Shape>();
            part->shape->createShape(TOshapes[i]);
            part->shape->measure();
            part->shape->init();

            part->localMin = part->shape->min;
            part->localMax = part->shape->max;

            part->updateBounds();

            obj->addChild(move(part)); // FIXME
        }
    }
}

void SceneInitializer::loadMapGeom(const std::string &resourceDirectory, const std::string &fileName, std::shared_ptr<GameObject> &obj)
{
    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    // load in the mesh and make the shape(s)
    // bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + fileName).c_str());
    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + fileName).c_str(), (resourceDirectory + "/scene/textures/").c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        obj = make_shared<GameObject>();

        for(int currObj = 0; currObj < TOshapes.size(); currObj++)
        {
            auto part = make_unique<GameObject>();

            part->shape = make_shared<Shape>();
            part->shape->createShape(TOshapes[currObj]);
            part->shape->measure();
            //part->shape->init();

            part->position = (part->shape->max + part->shape->min) / 2.0f;

            part->shape->init();

            part->localMin = part->shape->min;
            part->localMax = part->shape->max;

            // part->scale = part->shape->max - part->shape->min;

            part->updateBoundsMapGeom();

            part->objName = TOshapes[currObj].name;

            // Read normals from obj file
            const auto& meshNormals = TOshapes[currObj].mesh.normals;
            
            if (!meshNormals.empty())
            {
                for (int i = 0; i < meshNormals.size(); i+=3)
                {
                    // int normalIdx = TOshapes[0].mesh.indices;
                    glm::vec3 objNormal = glm::vec3(meshNormals[i + 0], meshNormals[i + 1], meshNormals[i + 2]);
                    part->normals.push_back(glm::normalize(objNormal));
                }
            }
            else
            {
                std::cout << "No normals in obj file" << std::endl;
                part->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            }

            // Material properties
            if(part->objName.find("blue") != string::npos) part->color = 1;
            if(part->objName.find("purple") != string::npos) part->color = 2;
            
            // Portal object
            if(part->objName.find("portal") != string::npos)
            {
                // Object is a quad / plane
                //part->portalID = part->objName[6]; // get last character / number of the portal
                if(part->objName.find("entrance1") != string::npos)
                    part->portalID = 1; // get last character / number of the portal
                else if(part->objName.find("exit1") != string::npos)
                    part->portalID = 2; // get last character / number of the portal
                //part->portalID = part->objName[6]; // get last character / number of the portal
                else if(part->objName.find("entrance2") != string::npos)
                    part->portalID = 3; // get last character / number of the portal
                else if(part->objName.find("exit2") != string::npos)
                    part->portalID = 4; // get last character / number of the portal
                
                part->source = part.get();
                portals.push_back(part.get());

                // FIXME currently breaks if entrance is after exit in obj file order
                if(part->portalID == 2)
                {
                    GameObject *first = portals[0];
                    GameObject *second = portals[1];

                    first->destination = second;
                    second->destination = first;
                }

                if(part->portalID == 4)
                {
                    GameObject *first = portals[2];
                    GameObject *second = portals[3];

                    first->destination = second;
                    second->destination = first;
                }

                //portals.insert({part->portalID, part});
                
                cout << "portalID: " << part->portalID << endl;

                // Calculate scale
                glm::vec3 scale = glm::vec3(std::max(0.01f, part->shape->max.x - part->shape->min.x),
                                            std::max(0.01f, part->shape->max.y - part->shape->min.y),
                                            std::max(0.01f, part->shape->max.z - part->shape->min.z));
                part->scale = scale;

                
                // Read normals from obj file
                glm::vec3 normal = part->normals[0];
                
                glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
                if (glm::abs(glm::dot(normal, worldUp)) > 0.99f) // FIXME CHECK THIS
                    worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
                
                glm::vec3 right = glm::normalize(glm::cross(worldUp, normal));
                
                glm::vec3 localUp = glm::normalize(glm::cross(normal, right)); // or forward depending on orientation
                
                part->rotationMat = glm::mat4(
                    glm::vec4(right, 0.0f),
                    glm::vec4(localUp, 0.0f),
                    glm::vec4(normal, 0.0f),
                    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
                );

            } else {
                part->portalID = 0;
            }
            
            obj->addChild(move(part)); // FIXME
        }
    }
}

void SceneInitializer::initGeom(const std::string &resourceDirectory)
{
    // Initialize mesh, Load geometry
    // Some obj files contain material information.We'll ignore them for this assignment.
    // load in the mesh and make the shape(s)

    loadGeom(resourceDirectory, "/objects/wedge.obj", player);
    loadGeom(resourceDirectory, "/objects/cube.obj", skybox); // use a cube map
    loadGeom(resourceDirectory, "/objects/cube.obj", cube);
    loadGeom(resourceDirectory, "/objects/plane.obj", plane);
    loadGeom(resourceDirectory, "/objects/angled_plane.obj", angledplane);
    loadGeom(resourceDirectory, "/objects/testcube.obj", testcube);
    loadGeom(resourceDirectory, "/objects/rocket-jumper.obj", tool);
    loadGeom(resourceDirectory, "/objects/cube.obj", projectile);
    loadGeom(resourceDirectory, "/objects/cube.obj", pawn);
    loadGeom(resourceDirectory, "/objects/wedge.obj", arrow);
    loadGeom(resourceDirectory, "/scene/Untitled.obj", mapGeomNoHier);
    //loadMapGeom(resourceDirectory, "/scene/testscenewithportal.obj", mapGeom);
    loadMapGeom(resourceDirectory, "/scene/Untitled.obj", mapGeom);

    // light

    // skybox
    skybox->scale = vec3(2000.0f);
    skybox->position = vec3(0.0f, -200.0f, 0.0f);

    projectile->scale = vec3(2.0f);

    pawn->scale = vec3(1.0f);

    player->scale = vec3(10.0f, 75.0f, 10.0f);
    
    plane->position = vec3(-100.0f, -10.0f, 10.0f);
    plane->scale = vec3(100.0f);

    angledplane->position = vec3(-100.0f, -10.0f, 10.0f);
    angledplane->scale = vec3(100.0f);
    
    testcube->position = vec3(0.0f, 10.0f, -50.0f);
    testcube->scale = vec3(50.0f);

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