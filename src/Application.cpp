// fixme dependencies
#include <iostream>
#include <glad/glad.h>
#include "core/GLSL.h"
#include "core/GLSLUtils.h"
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Application.h"
#include "SceneInitializer.h"
#include "SceneRender.h"
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

void Application::init(const string &resourceDirectory)
{
    // FIXME
    scene = make_shared<SceneInitializer>();
    sceneRender = make_shared<SceneRender>();
    callbacks = make_shared<Callbacks>(scene->mainCamera.get());
	scene->init(resourceDirectory);
	scene->initGeom(resourceDirectory);
}

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

void Application::render(float frametime)
{
    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    // Use the matrix stack for Lab 6
    float aspect = width / (float)height;

    // Create the matrix stacks
    auto Projection = make_shared<MatrixStack>();
    auto ProjectionPortal = make_shared<MatrixStack>();
    auto Model = make_shared<MatrixStack>();
    auto ModelPortalSource = make_shared<MatrixStack>();
    auto ModelPortalDestination = make_shared<MatrixStack>();
    //ModelPortalSource->translate(scene->portalEntrancePos);
    //ModelPortalDestination->translate(scene->portalExitPos);

    // Apply perspective projection.
    Projection->pushMatrix();
    Projection->perspective(45.0f, aspect, 0.01f, 1000.0f); // FIXME, was 100
    ProjectionPortal->pushMatrix();
    ProjectionPortal->perspective(45.0f, aspect, 0.01f, 1000.0f); // FIXME, was 100

    // CREATE PORTAL MATRICES
    scene->portalEntranceDoor->updateBounds();
    scene->portalEntranceDoor->rotation = vec3(0, 1, 0);
    scene->portalEntranceDoor->scale = vec3(1.0, 5.0, 5.0);
    scene->portalEntranceDoor->position = scene->portalEntrancePos;
    scene->portalEntranceDoor->angle = g_Spin * glfwGetTime();

    scene->portalExitDoor->updateBounds();
    scene->portalExitDoor->rotation = vec3(0, 1, 0);
    scene->portalExitDoor->scale = scene->portalEntranceDoor->scale;
    scene->portalExitDoor->position = scene->portalExitPos;
    scene->portalExitDoor->angle = g_Spin * glfwGetTime();

    // update matrices
    scene->skybox->rotation = vec3(0, 1, 0);
    //scene->skybox->angle = g_Spin * glfwGetTime();

    ModelPortalSource->loadIdentity();
    ModelPortalSource->translate(scene->portalEntranceDoor->position);

    ModelPortalDestination->loadIdentity();
    ModelPortalDestination->translate(scene->portalExitDoor->position);

    // FIXME move camerea functions here <<<
    // update the camera position
    // mainCamera->updateUsingCameraPath(frametime, splinepath);

    // Setup stencil buffers to draw portal objects
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear framebuffer and stencilbuffer
    // glStencilMask(0x00);               // each bit ends up as 0 in the stencil buffer (disabling writes)
    // glStencilFunc(GL_ALWAYS, 1, 0xFF); // only draw the 1 from the stencil buffer
    // glStencilMask(0xFF);               // each bit is written to the stencil buffer as is

    glStencilFunc(GL_ALWAYS, 1, 0xFF);    // only draw the 1 from the stencil buffer
    glStencilMask(0xFF);                  // each bit is written to the stencil buffer as is

    // Draw portal ENTRANCE frame
    scene->prog->bind();
        //scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, ModelPortalSource, ModelPortalDestination);
        scene->mainCamera->SetView(scene->prog);

        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
        //scene->mainCamera->SetPortalView(scene->prog, scene->mainCamera, ModelPortalSource, ModelPortalDestination);
        
        ModelPortalSource->pushMatrix();
            //ModelPortalSource->loadIdentity();

            //scene->portalCamera->eye = scene->portalExitPos;

            sceneRender->drawMesh(scene->prog, ModelPortalSource, scene->portalEntranceDoor, 1);
        ModelPortalSource->popMatrix();
    scene->prog->unbind();

    // Draw portal EXIT frame
    scene->prog->bind();
        scene->mainCamera->SetView(scene->prog);    

        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
        
        ModelPortalDestination->pushMatrix();
            //ModelPortalDestination->loadIdentity();

            sceneRender->drawMesh(scene->prog, ModelPortalDestination, scene->portalExitDoor, 2);
        ModelPortalDestination->popMatrix();
    scene->prog->unbind();

    // Setup stencil buffer to draw other objects over the portal
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    // DRAW BORDER OBJECTS HERE
    // scene->prog->bind();
    // // set up all the  matrices
    // glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    // scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, ModelPortalSource, ModelPortalDestination);
    // //scene->mainCamera->SetView(scene->prog); // CHANGE TO PORTAL CAMERA FOR DIFFERENT RESULTS
    // // portalCamera->SetView(prog);
    // //  glUniform3f(prog->getUniform("lightPos"), 2.0 + callbacks->lightTrans, 2.0, 2.9);
    // glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

    //     Model->pushMatrix();
    //         Model->loadIdentity();
    //         // update matrices
    //         scene->skybox->updateBounds();

    //         ModelPortalDestination->translate(scene->skybox->position + glm::vec3(5.0f, 1.0, 5.0));
    //         ModelPortalDestination->translate(scene->skybox->position); // move to ground (half of height)
    //         ModelPortalDestination->rotate(scene->skybox->angle, scene->skybox->rotation);
    //         ModelPortalDestination->scale(1.0 / scene->skybox->shape->largeExtent() + 0.25); // normalize

    //         GLSLUtils::SetMaterial(scene->prog, 2);
    //         GLSLUtils::setModel(scene->prog, ModelPortalDestination);
    //         scene->skybox->shape->draw(scene->prog);
    //     Model->pushMatrix();
    // scene->prog->unbind();

    // Reset stencil buffer state
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    // enable drawing only on stencil buffer 1's
    glStencilFunc(GL_LEQUAL, 1, 0xFF);

    // REDRAW SCENE IN PORTAL - Redraw scene but with portal view (portal camera)
    scene->prog->bind();
        scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, ModelPortalSource, ModelPortalDestination);

        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        Model->pushMatrix();
            Model->loadIdentity();
            sceneRender->drawMesh(scene->prog, Model, scene->skybox, 0);
        Model->pushMatrix();
    scene->prog->unbind();

    // Reset stencil buffer state
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);

    // --- DRAW MAIN CAMERA SCENE ---
    /*
    // use the texture shader
    scene->texProg->bind();
    glUniformMatrix4fv(scene->texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    scene->mainCamera->SetView(scene->texProg);
    glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
    glUniform1f(scene->texProg->getUniform("MatShine"), 27.9);
    glUniform1i(scene->texProg->getUniform("flip"), 1);
    scene->texture1->bind(scene->texProg->getUniform("Texture0"));
    
    glUniform1i(scene->texProg->getUniform("flip"), 0);
    /*
    // drawGround(texProg);
    // drawSkybox(texProg, Model, skybox);
    // drawHierMap(texProg, Model, scene); // FIXME, SCALE IS WRONG

    // Model->rotate(g_Spin * glfwGetTime(), vec3(0, -1, 0));a

    /*
    // timer to create new arrow
    if (timer > 0.0f)
    {
        timer -= frametime;
    }
    else
    {
        auto arrowChild = make_unique<GameObject>(
            scene->arrow->shape,
            vec3(rand() % 10 + 2, 0.0f, rand() % 10 + 2),
            0.0f,
            vec3(0.0f, radians((float)(rand() % 360)), 0.0f),
            vec3(1.0f),
            scene->arrow->localMin,
            scene->arrow->localMax);
        arrowChild->velocity = vec3(rand() % 10, 0, rand() % 10);

        arrowChild->updateBounds();
        scene->arrow->addChild(std::move(arrowChild));
        timer = 1.0f; // reset timer
        scene->objectCount += 1;
        cout << "Timer is finished, adding another object";
    }

    // iterate over each child of arrow
    for (auto &arrowChild : scene->arrow->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        // physics updates
        if (arrowChild->collided > 10)
        {
            arrowChild->position = vec3(0);
        }
        else if (arrowChild->collided > 5 || arrowChild->cameraCollided)
        {
            arrowChild->position += vec3(5 * sin(5 * glfwGetTime()), 1.0, 0.0) * deltaTime;
        }
        else
        {
            arrowChild->position += arrowChild->velocity * deltaTime;
        }

        // change rotation to be in direction of velocity
        vec3 forward = -normalize(arrowChild->velocity); // normalize to velocity vector and flip
        vec3 right = normalize(cross(vec3(0, 1, 0), forward));
        vec3 up = cross(forward, right);

        mat4 rotationMat(1.0f);
        rotationMat[0] = vec4(right, 0);   // column 1, x axis
        rotationMat[1] = vec4(up, 0);      // column 2, y axis
        rotationMat[2] = vec4(forward, 0); // column 3, z axis
        rotationMat[3] = vec4(vec3(0), 1); // column 4, w axis

        arrowChild->updateBounds();

        // Model->translate(arrow->getChild() + velocity));
        Model->translate(arrowChild->position);
        Model->multMatrix(rotationMat);
        Model->scale(1.0 / arrowChild->shape->largeExtent());

        GLSLUtils::setModel(scene->texProg, Model);
        // if(!arrowChild->isMarked) arrowChild->shape->draw(texProg);

        Model->popMatrix();
    }
    scene->texProg->unbind();
    */

    // use the material shader
    scene->prog->bind();
        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        scene->mainCamera->SetView(scene->prog);
        glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));


    // DRAW WALLS
    for (auto &cubeChild : scene->cube->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        cubeChild->updateBounds();
        Model->translate(cubeChild->position);
        Model->scale(cubeChild->scale);
        Model->scale(1.0 / cubeChild->shape->largeExtent());

        GLSLUtils::SetMaterial(scene->prog, 1);
        GLSLUtils::setModel(scene->prog, Model);
        cubeChild->shape->draw(scene->prog);

        Model->popMatrix();
    }
    scene->prog->unbind();

    scene->prog->bind();
        scene->mainCamera->SetView(scene->prog);

        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        Model->pushMatrix();
            Model->loadIdentity();

            sceneRender->drawMesh(scene->prog, Model, scene->skybox, 3);
        Model->pushMatrix();
    scene->prog->unbind();

    // --- CAMERA AND COLLISION LOGIC --- FIXME
    // animation updates
    sTheta = sin(glfwGetTime());
    eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
    hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

    // save previous camera position for collision
    scene->mainCamera->eye_prev = scene->mainCamera->eye;

    // camera movements
    if (callbacks->cinematicCamera)
        scene->mainCamera->updateUsingCameraPath(deltaTime, scene->splinepath);
    if (callbacks->freeCamera)                                                          // FIXME MOVE TO BEGINNING
        scene->mainCamera->cameraMovement(windowManager->getHandle(), scene->cameraSpeed, deltaTime); // smooth camera movements
    // else
    //	playerMovement(windowManager->getHandle(), 3.0); // control the player

    scene->mainCamera->lookAtTarget = scene->mainCamera->eye + scene->mainCamera->forward; // FIXME, put this before?

    /*
    // -- COLLISION CHECKING --- FIXME / TODO PUT THIS IN ANOTHER CLASS
    scene->skybox->collided = AABB::intersectsCamera(*scene->mainCamera, *scene->skybox);

    for (auto &arrowChild : scene->arrow->children)
    {
        if (AABB::intersectsCamera(*scene->mainCamera, *arrowChild) != 0)
        {
            if (arrowChild->collisionsEnabled > 0)
            {
                arrowChild->cameraCollided += 1;
                arrowChild->collided += 1;
                arrowChild->isMarked = true;
                scene->objectCollisionCount += 1;
                scene->objectCount -= 1;
            }
            arrowChild->collisionsEnabled = 0;
            break;
        }
    }

    for (auto &cubeChild : scene->cube->children)
    {
        if (AABB::intersectsCamera(*scene->mainCamera, *cubeChild) != 0)
        {
            cubeChild->collided = 1;
            break;
        }
    }

    // check if arrow hits wall
    for (auto &arrowChild : scene->arrow->children)
    {
        for (auto &cubeChild : scene->cube->children)
        {
            if (AABB::intersectsObject(*arrowChild, *cubeChild) != 0)
            {
                arrowChild->velocity = -arrowChild->velocity; // reverse direction;
                arrowChild->collided = 1;
                cubeChild->collided = 1;

                arrowChild->cameraCollided = 0;
                arrowChild->collided = 0;
                break;
            }
        }
    }

    // check if arrow hits another arrow
    for (auto &arrowChild1 : scene->arrow->children)
    {
        for (auto &arrowChild2 : scene->arrow->children)
        {
            if (arrowChild1 != arrowChild2 && AABB::intersectsObject(*arrowChild1, *arrowChild2) != 0)
            {
                vec3 tmp = arrowChild1->velocity;
                arrowChild1->velocity = -arrowChild2->velocity; // reverse direction
                arrowChild2->velocity = -arrowChild1->velocity; // reverse direction
                arrowChild1->collided = 1;
                arrowChild2->collided = 1;
                // break;
            }
        }
    }

    scene->debugShader->bind();
    // Set global matrices FIXME-COMMENT
    glUniformMatrix4fv(scene->debugShader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
    scene->mainCamera->SetView(scene->debugShader);

    glUniform3fv(scene->debugShader->getUniform("camPos"), 1, value_ptr(scene->mainCamera->eye));
    glUniform4f(scene->debugShader->getUniform("debugColor"), 0.0f, 0.0f, 1.0f, 0.5f);

    // Set model (M) matrix
    Model->pushMatrix();
    Model->loadIdentity();

    glUniform3fv(scene->debugShader->getUniform("boxMin"), 1, value_ptr(scene->skybox->min));
    glUniform3fv(scene->debugShader->getUniform("boxMax"), 1, value_ptr(scene->skybox->max));
    glUniform1i(scene->debugShader->getUniform("collided"), scene->skybox->collided);

    Model->translate(scene->skybox->position);
    // Model->rotate(1.0f, skybox->rotation); // FIXME
    Model->scale(1.0f / scene->skybox->shape->largeExtent());
    Model->scale(1.1f);

    // draw wireframe hitbox
    GLSLUtils::setModel(scene->debugShader, Model);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene->skybox->shape->draw(scene->debugShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Model->popMatrix();

    // glUniform4f(debugShader->getUniform("debugColor"), 0.0f, 0.0f, 1.0f, 0.5f);

    for (auto &arrowChild : scene->arrow->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        Model->translate(arrowChild->position);
        // Model->rotate(1, arrowChild->rotation);
        Model->scale(1.0f / arrowChild->shape->largeExtent());
        Model->scale(vec3(1.1f));

        glUniform3fv(scene->debugShader->getUniform("boxMin"), 1, value_ptr(arrowChild->min));
        glUniform3fv(scene->debugShader->getUniform("boxMax"), 1, value_ptr(arrowChild->max));
        glUniform1i(scene->debugShader->getUniform("collided"), arrowChild->collided);

        GLSLUtils::setModel(scene->debugShader, Model);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        arrowChild->shape->draw(scene->debugShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        Model->popMatrix();
    }

    for (auto &cubeChild : scene->cube->children)
    {
        Model->pushMatrix();
        Model->loadIdentity();

        cubeChild->updateBounds();

        Model->translate(cubeChild->position);
        // Model->rotate(1, cubeChild->rotation);
        Model->scale(cubeChild->scale + vec3(0.1f));
        Model->scale(1.0f / cubeChild->shape->largeExtent());

        glUniform3fv(scene->debugShader->getUniform("boxMin"), 1, value_ptr(cubeChild->min));
        glUniform3fv(scene->debugShader->getUniform("boxMax"), 1, value_ptr(cubeChild->max));
        glUniform1i(scene->debugShader->getUniform("collided"), cubeChild->collided);

        GLSLUtils::setModel(scene->debugShader, Model);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        cubeChild->shape->draw(scene->debugShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        Model->popMatrix();
    }
    scene->debugShader->unbind();
    */
}