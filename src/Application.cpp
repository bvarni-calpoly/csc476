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
    //FIXME, DO NOT SET EVERY FRAME
	// Does not work on WSL
    if(callbacks->mouseEnabled) glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    // Use the matrix stack for Lab 6
    float aspect = width / (float)height;

    // Create the matrix stacks
    //auto Projection = make_shared<MatrixStack>();
    //auto ProjectionPortal = make_shared<MatrixStack>();
    //auto Model = make_shared<MatrixStack>();
    //auto ModelPortalSource = make_shared<MatrixStack>();
    //auto ModelPortalDestination = make_shared<MatrixStack>();

    scene->Projection = make_shared<MatrixStack>();
    scene->ProjectionPortal = make_shared<MatrixStack>();
    scene->Model = make_shared<MatrixStack>();
    scene->ModelPortalSource = make_shared<MatrixStack>();
    scene->ModelPortalDestination = make_shared<MatrixStack>();

    // Apply perspective projection.
    scene->Projection->pushMatrix();
    scene->Projection->perspective(45.0f, aspect, 0.01f, 3000.0f); // FIXME, was 100
    scene->ProjectionPortal->pushMatrix();
    scene->ProjectionPortal->perspective(45.0f, aspect, 0.01f, 3000.0f); // FIXME, was 100

    // update matrices
    scene->skybox->rotation = vec3(0, 1, 0);
    scene->skybox->angle = g_Spin * glfwGetTime();

    scene->ModelPortalSource->loadIdentity();
    scene->ModelPortalDestination->loadIdentity();

    // FIXME move camerea functions here <<<
    // update the camera position
    // mainCamera->updateUsingCameraPath(frametime, splinepath);
    
    // Recursive portals
    // https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear framebuffer and stencilbuffer
    glm::mat4 mainView = lookAt(scene->mainCamera->eye, scene->mainCamera->lookAtTarget, glm::vec3(0, 1, 0)); // TC, lookAt returns view matrix
    //sceneRender->drawRecursivePortals(scene, sceneRender, callbacks, mainView, scene->Projection, 1, 0);
    sceneRender->drawPortals(scene, sceneRender, callbacks, mainView, scene->Projection->topMatrix());
    sceneRender->drawTool(scene, sceneRender, callbacks, mainView, scene->Projection->topMatrix());
    
    /*
    // DRAW BORDER OBJECTS HERE

    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilMask(0x00);
    // glDisable(GL_DEPTH_TEST);
    // Reset stencil buffer state
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    // enable drawing only on stencil buffer 1's

    glStencilFunc(GL_LEQUAL, 1, 0xFF);

    // Reset stencil buffer state
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
    */
    
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
    else
        scene->mainCamera->playerMovement(windowManager->getHandle(), scene, 175.0, deltaTime); // control the player

    scene->mainCamera->lookAtTarget = scene->mainCamera->eye + scene->mainCamera->forward; // FIXME, put this before?

    /*
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