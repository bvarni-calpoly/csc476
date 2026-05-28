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

void RenderQuad(unsigned int &quadVAO, unsigned int &quadVBO) // FIXME REMOVE THIS
{
    // https://learnopengl.com/getting-started/hello-triangle
    // https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/5.1.framebuffers/framebuffers.cpp

    // static unsigned int quadVAO = 0;
    // static unsigned int quadVBO = 0;

    // If the quad hasn't been generated yet, set it up on the GPU
    if (quadVAO == 0)
    {
        // 4 vertices, each with 3 position floats (X, Y, Z) and 2 texture coordinate floats (U, V)
        // This matches layout(location = 0) for positions and layout(location = 2) for texCoords
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };

        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        // Location 0: Vertex Positions (X, Y, Z)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);

        // Location 2: Texture Coordinates (U, V) - Matches your layout(location = 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }

    // Bind and Draw the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Application::render(float frametime)
{
    // FIXME, DO NOT SET EVERY FRAME
    //  Does not work on WSL
    if (callbacks->mouseEnabled)
        glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    // Use the matrix stack for Lab 6
    float aspect = width / (float)height;

    // Create the matrix stacks
    scene->Projection = make_shared<MatrixStack>();
    scene->ProjectionPortal = make_shared<MatrixStack>();
    scene->Model = make_shared<MatrixStack>();
    scene->ModelPortalSource = make_shared<MatrixStack>();
    scene->ModelPortalDestination = make_shared<MatrixStack>();

    // Apply perspective projection.
    scene->Projection->pushMatrix();
    // scene->Projection->perspective(45.0f, aspect, 0.01f, 2000.0f);
    // FIXME
    float FOV = 45.0f;
    FOV += glm::length(scene->playerCamera->velocity) / 1000.0f;

    scene->Projection->perspective(FOV, aspect, 0.1f, 5000.0f); // FIXME, was 100
    scene->ProjectionPortal->pushMatrix();
    scene->ProjectionPortal->perspective(45.0f, aspect, 0.01f, 1000.0f); // FIXME, was 100

    // update matrices
    scene->skybox->rotation = vec3(0, 1, 0);
    scene->skybox->angle = g_Spin * glfwGetTime();

    scene->ModelPortalSource->loadIdentity();
    scene->ModelPortalDestination->loadIdentity();

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
    if (callbacks->freeCamera)                                                                        // FIXME MOVE TO BEGINNING
        scene->mainCamera->cameraMovement(windowManager->getHandle(), scene->cameraSpeed, deltaTime); // smooth camera movements
    else
        scene->playerCamera->playerMovement(windowManager->getHandle(), scene, 200.0, deltaTime); // control the player

    scene->mainCamera->lookAtTarget = scene->mainCamera->eye + scene->mainCamera->forward; // FIXME, put this before?

    // https://learnopengl.com/Advanced-OpenGL/Framebuffers
    // first pass
    // glBindFramebuffer(GL_FRAMEBUFFER, scene->framebuffer); // bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, scene->hdrFBO); // bind framebuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glEnable(GL_DEPTH_TEST);

    // Recursive portals
    // https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);                               // Clear framebuffer and stencilbuffer
    glm::mat4 mainView = lookAt(scene->mainCamera->eye, scene->mainCamera->lookAtTarget, glm::vec3(0, 1, 0)); // TC, lookAt returns view matrix
    // sceneRender->drawRecursivePortals(scene, sceneRender, callbacks, mainView, scene->Projection, 1, 0);
    sceneRender->drawPortals(scene, sceneRender, callbacks, mainView, scene->Projection->topMatrix());
    sceneRender->drawTool(scene, sceneRender, callbacks, mainView, deltaTime);

    // https://learnopengl.com/Advanced-OpenGL/Framebuffers
    // gaussian blur pass
    bool horizontal = true, first_iteration = true;
    int amount = 10;
    scene->blurShader->bind();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, scene->pingpongFBO[horizontal]);
        glUniform1i(scene->blurShader->getUniform("horizontal"), horizontal);
        glActiveTexture(GL_TEXTURE0); // activate texture 0 to read from it
        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? scene->colorBuffers[1] : scene->pingpongBuffers[!horizontal]);

        glUniform1i(scene->blurShader->getUniform("image"), 0); // send texture to blur shader

        RenderQuad(scene->quadVAO, scene->quadVBO);

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    scene->blurShader->unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    scene->screenShader->bind();
    // FIX ME
    glDisable(GL_DEPTH_TEST);

    // Bind scene texture 0
    glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, scene->textureColorbuffer); // post processing framebuffer
    glBindTexture(GL_TEXTURE_2D, scene->colorBuffers[0]);
    glUniform1i(scene->screenShader->getUniform("sceneTexture"), 0);

    // Bind bright object texture 1
    glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, scene->colorBuffers[1]); // bright object framebuffer
    glBindTexture(GL_TEXTURE_2D, scene->pingpongBuffers[!horizontal]); // blur bright object framebuffer
    glUniform1i(scene->screenShader->getUniform("bloomBlurTexture"), 1);

    glUniform1f(scene->screenShader->getUniform("exposure"), 1.0f);

    // Send texture to shader
    // scene->texture0->bind(scene->screenShader->getUniform("Texture0"));

    // glDrawArrays(GL_TRIANGLES, 0, 6);

    RenderQuad(scene->quadVAO, scene->quadVBO);

    scene->screenShader->unbind();

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
}