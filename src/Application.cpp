// fixme dependencies
#include <iostream>
#include <glad/glad.h>
#include "core/GLSL.h"
#include "core/GLSLUtils.h"
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Application.h"
#include "core/WindowManager.h"
#include "renderer/Program.h"
#include "renderer/Shape.h"
#include "renderer/MatrixStack.h"
#include "renderer/Texture.h"
#include "../ext/stb_image/stb_image.h"
#include "math/Bezier.h"
#include "math/Spline.h"
#include "core/Callbacks.h"
#include "world/Camera.h"
#include "world/Player.h"
#include "world/GameObject.h"
#include "physics/AABB.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// imgui FIXME
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"

#define PI 3.1415927

using namespace std;
using namespace glm;

Application::Application(float dt) : deltaTime(dt) {};

Application::~Application() {}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (callbacks)
        callbacks->keyCallback(window, key, scancode, action, mods);
}

void Application::mouseCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (callbacks)
        callbacks->mouseCallback(window, button, action, mods);
}

void Application::scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
{
    if (callbacks)
        callbacks->scrollCallback(window, deltaX, deltaY);
}

void Application::setCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    // Prevent camera moving when hovering over imgui window
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    // toggle mouse movement
    if (callbacks->mouseEnabled)
        if (callbacks)
            callbacks->setCursorPosCallback(window, xpos, ypos);
}

void Application::resizeCallback(GLFWwindow *window, int width, int height)
{
    if (callbacks)
        callbacks->resizeCallback(window, width, height);
}

void Application::init(const std::string &resourceDirectory)
{
    GLSL::checkVersion();

    // setup callbacks
    callbacks = make_shared<Callbacks>(mainCamera.get());

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

void Application::initGeom(const std::string &resourceDirectory)
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
        scene = make_shared<GameObject>();

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

            scene->addChild(move(part));
        }
    }

    // Initialize skybox mesh
    vector<tinyobj::shape_t> TOshapesSkybox;
    vector<tinyobj::material_t> objMaterialsSkybox;
    // load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapesSkybox, objMaterialsSkybox, errStr, (resourceDirectory + "/objects/cube.obj").c_str());
    if (!rc)
    {
        cerr << errStr << endl;
    }
    else
    {
        skybox = make_shared<GameObject>();
        skybox->shape = make_shared<Shape>();
        skybox->shape->createShape(TOshapesSkybox[0]);
        skybox->shape->measure();
        skybox->shape->init();

        skybox->localMin = skybox->shape->min;
        skybox->localMax = skybox->shape->max;

        skybox->updateBounds(); // FIXME
    }

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
            vec3(rand() % 10, 0.0f, rand() % 10),
            0.0f,
            vec3(0.0f, radians((float)(rand() % 360)), 0.0f),
            vec3(1.0f),
            arrow->localMin,
            arrow->localMax);
        arrowChild->velocity = vec3(0.5, 0.0, 0.0);

        arrowChild->updateBounds();
        arrow->addChild(std::move(arrowChild));
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
    cube->children[0]->position = vec3(0.0f, 0.0f, wallLength/2);

    cube->children[1]->scale = vec3(wallLength, 1.0f, 1.0f);
    cube->children[1]->position = vec3(wallLength/2, 0.0f, 0.0f);

    cube->children[2]->scale = vec3(1.0f, 1.0f, wallLength);
    cube->children[2]->position = vec3(wallLength, 0.0f, wallLength/2);

    cube->children[3]->scale = vec3(wallLength, 1.0f, 1.0f);
    cube->children[3]->position = vec3(wallLength/2, 0.0f, wallLength);
}

// directly pass quad for the ground to the GPU
void Application::initGround()
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

// code to draw the ground plane
void Application::drawGround(shared_ptr<Program> curS)
{
    curS->bind();
    glBindVertexArray(GroundVertexArrayID);
    texture0->bind(curS->getUniform("Texture0"));
    // draw the ground plane
    GLSLUtils::SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // draw!
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    curS->unbind();
}

void Application::drawSkybox(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<Shape> shape)
{
    curS->bind();
    glUniform1i(curS->getUniform("flip"), 0);
    texture1->bind(curS->getUniform("Texture0"));

    /*
    skyboxObject[0] = GameObject( // fixme
        skybox,
        vec3(0),
        vec3(0),
        vec3(0),
        skybox->min,
        skybox->max
    );
    */

    // skyboxObject[0].updateBounds();

    // draw the skybox
    Model->pushMatrix();
    Model->loadIdentity();

    float halfHeight = (shape->max.y - shape->min.y) / 2.0f;

    Model->scale(vec3(50.0f));
    Model->rotate(g_Spin * glfwGetTime(), vec3(0, -1, 0));
    Model->translate(vec3(0, -0.005f, 0)); // translate down 0.01 (so ground shows up)

    // normalize
    Model->scale(1.0 / shape->largeExtent());
    Model->translate(vec3(0, halfHeight, 0)); // move to ground (half of height)
    Model->translate(-shape->center);         // move to origin

    GLSLUtils::setModel(curS, Model);
    shape->draw(curS);
    Model->popMatrix();
    curS->unbind();
}

// draw map / scene / level / scene
void Application::drawHierMap(shared_ptr<Program> curS, shared_ptr<MatrixStack> model, vector<shared_ptr<Shape>> shape)
{
    model->pushMatrix();
    model->loadIdentity();

    // SRT
    // model->scale(1.0/shape->largeExtent());
    // model->rotate();
    // model->translate();
    model->scale(0.25);
    for (auto part : shape)
    {
        GLSLUtils::setModel(curS, model);
        part->draw(curS);
    }

    model->popMatrix();
}

void Application::render(float frametime)
{
    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    // Clear framebuffer.
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // FIXME STENCIL BUFFERS https://learnopengl.com/Advanced-OpenGL/Stencil-testing
    // Stencil buffers
    glEnable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear framebuffer and stencilbuffer

    // glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // only draw the 1 from the stencil buffer
    glStencilMask(0xFF);               // each bit is written to the stencil buffer as is

    // READ ME <<<
    // https://ajknowles11.github.io/projects/portals/
    // https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
    // https://www.youtube.com/watch?v=cWpFZbjtSQg
    // https://medium.com/@alikomurcu/portals-with-opengl-d74da6241dd4

    // END OF FIXME FOR STENCIL BUFFERS

    // Use the matrix stack for Lab 6
    float aspect = width / (float)height;

    // Create the matrix stacks - please leave these alone for now
    auto Projection = make_shared<MatrixStack>();
    auto Model = make_shared<MatrixStack>();

    // FIXME move camerea functions here <<<

    // update the camera position
    // mainCamera->updateUsingCameraPath(frametime, splinepath);

    // Apply perspective projection.
    Projection->pushMatrix();
    Projection->perspective(45.0f, aspect, 0.01f, 1000.0f); // FIXME, was 100

    // use the texture shader
    texProg->bind();
    glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    mainCamera->SetView(texProg);
    glUniform3fv(texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
    glUniform1f(texProg->getUniform("MatShine"), 27.9);
    glUniform1i(texProg->getUniform("flip"), 1);
    texture1->bind(texProg->getUniform("Texture0"));

    glUniform1i(texProg->getUniform("flip"), 0);
    // drawGround(texProg);
    // drawSkybox(texProg, Model, skybox);
    // drawHierMap(texProg, Model, scene); // FIXME, SCALE IS WRONG

    // Model->rotate(g_Spin * glfwGetTime(), vec3(0, -1, 0));a

    vec3 velocity = vec3(0);
    // iterate over each child of arrow
    for (auto &arrowChild : arrow->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        // Model->translate(arrow->getChild() + velocity));
        Model->translate(arrowChild->position);
        Model->scale(1.0 / arrowChild->shape->largeExtent());

        GLSLUtils::setModel(texProg, Model);
        arrowChild->shape->draw(texProg);

        Model->popMatrix();
    }

    texProg->unbind();

    // STENCIL AGAIN
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    // END STENCIL TEST

    // use the material shader
    prog->bind();
    // set up all the matrices
    glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    mainCamera->SetView(prog);
    // glUniform3f(prog->getUniform("lightPos"), 2.0 + callbacks->lightTrans, 2.0, 2.9);
    glUniform3fv(texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
    Model->pushMatrix();
    Model->loadIdentity();

    // update matrices
    skybox->updateBounds();
    skybox->rotation = vec3(0, -1, 0);
    skybox->angle = g_Spin * glfwGetTime();

    Model->translate(skybox->position); // move to ground (half of height)
    Model->rotate(skybox->angle, skybox->rotation);
    Model->scale(1.0 / skybox->shape->largeExtent() + 0.1); // normalize

    GLSLUtils::SetMaterial(prog, 1);
    GLSLUtils::setModel(prog, Model);
    skybox->shape->draw(prog);
    Model->popMatrix();

    // DRAW WALLS
    for (auto &cubeChild : cube->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        cubeChild->updateBounds();
        Model->translate(cubeChild->position);
        Model->scale(cubeChild->scale);
        Model->scale(1.0 / cubeChild->shape->largeExtent());

        GLSLUtils::SetMaterial(prog, 1);
        GLSLUtils::setModel(prog, Model);
        cubeChild->shape->draw(prog);

        Model->popMatrix();
    }

    prog->unbind();

    // --- CAMERA AND COLLISION LOGIC --- FIXME
    // animation updates
    sTheta = sin(glfwGetTime());
    eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
    hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

    // save previous camera position for collision
    mainCamera->eye_prev = mainCamera->eye;

    // camera movements
    if (callbacks->cinematicCamera)
        mainCamera->updateUsingCameraPath(deltaTime, splinepath);
    if (callbacks->freeCamera)                                                  // FIXME MOVE TO BEGINNING
        mainCamera->cameraMovement(windowManager->getHandle(), 3.0, deltaTime); // smooth camera movements
    // else
    //	playerMovement(windowManager->getHandle(), 3.0); // control the player

    mainCamera->lookAtTarget = mainCamera->eye + mainCamera->forward; // FIXME, put this before?

    // -- COLLISION CHECKING --- FIXME / TODO PUT THIS IN ANOTHER CLASS
    skybox->collided = AABB::intersectsCamera(*mainCamera, *skybox);

    for (auto &arrowChild : arrow->children)
    {
        if (AABB::intersectsCamera(*mainCamera, *arrowChild) != 0)
        {
            arrowChild->collided += 1;
            break;
        }
    }

    for (auto &cubeChild : cube->children)
    {
        if (AABB::intersectsCamera(*mainCamera, *cubeChild) != 0)
        {
            cubeChild->collided = 1;
            break;
        }
    }

    // check if arrow hits wall
    for (auto &arrowChild : arrow->children)
    {
        for (auto &cubeChild : cube->children)
        {
            if (AABB::intersectsObject(*arrowChild, *cubeChild) != 0)
            {
                arrowChild->collided = 1;
                cubeChild->collided = 1;
                break;
            }
        }
    }

    debugShader->bind();
    // Set global matrices FIXME-COMMENT
    glUniformMatrix4fv(debugShader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    mainCamera->SetView(debugShader);

    glUniform3fv(debugShader->getUniform("camPos"), 1, value_ptr(mainCamera->eye));
    glUniform4f(debugShader->getUniform("debugColor"), 0.0f, 0.0f, 1.0f, 0.5f);

    // Set model (M) matrix
    Model->pushMatrix();
    Model->loadIdentity();


    glUniform3fv(debugShader->getUniform("boxMin"), 1, value_ptr(skybox->min));
    glUniform3fv(debugShader->getUniform("boxMax"), 1, value_ptr(skybox->max));
    glUniform1i(debugShader->getUniform("collided"), skybox->collided);

    Model->translate(skybox->position);
    // Model->rotate(1.0f, skybox->rotation); // FIXME
    Model->scale(1.0f / skybox->shape->largeExtent());
    Model->scale(1.1f);

    // draw wireframe hitbox
    GLSLUtils::setModel(debugShader, Model);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    skybox->shape->draw(debugShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Model->popMatrix();

    // glUniform4f(debugShader->getUniform("debugColor"), 0.0f, 0.0f, 1.0f, 0.5f);

    for (auto &arrowChild : arrow->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();
        // physics updates
        if (arrowChild->collided > 5)
        {
            arrowChild->position = vec3(0);
        }
        else if (arrowChild->collided > 0)
        {
            arrowChild->position += vec3(5 * sin(5 * glfwGetTime()), 1.0, 0.0) * deltaTime;
        }
        else
        {
            arrowChild->position += arrowChild->velocity * deltaTime;
        }
        arrowChild->updateBounds();

        Model->translate(arrowChild->position);
        // Model->rotate(1, arrowChild->rotation);
        Model->scale(1.0f / arrowChild->shape->largeExtent());
        Model->scale(vec3(1.1f));

        glUniform3fv(debugShader->getUniform("boxMin"), 1, value_ptr(arrowChild->min));
        glUniform3fv(debugShader->getUniform("boxMax"), 1, value_ptr(arrowChild->max));
        glUniform1i(debugShader->getUniform("collided"), arrowChild->collided);

        GLSLUtils::setModel(debugShader, Model);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        arrowChild->shape->draw(debugShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        Model->popMatrix();
    }

    for (auto &cubeChild : cube->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        cubeChild->updateBounds();

        Model->translate(cubeChild->position);
        // Model->rotate(1, cubeChild->rotation);
        Model->scale(cubeChild->scale + vec3(0.1f));
        Model->scale(1.0f / cubeChild->shape->largeExtent());

        glUniform3fv(debugShader->getUniform("boxMin"), 1, value_ptr(cubeChild->min));
        glUniform3fv(debugShader->getUniform("boxMax"), 1, value_ptr(cubeChild->max));
        glUniform1i(debugShader->getUniform("collided"), cubeChild->collided);

        GLSLUtils::setModel(debugShader, Model);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        cubeChild->shape->draw(debugShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        Model->popMatrix();
    }
    debugShader->unbind();

    // STENCIL FIX ME AGAIN!
    glStencilMask(0xFF);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // END OF FIXME
}