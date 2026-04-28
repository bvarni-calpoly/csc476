#include <glad/glad.h>
#include "SceneRender.h"
#include "core/GLSLUtils.h"
#include "core/Callbacks.h"
#include "world/GameObject.h"
#include "SceneInitializer.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

SceneRender::SceneRender(/* args */) {}

SceneRender::~SceneRender() {}

// code to draw the ground plane
void SceneRender::drawGround(shared_ptr<Program> curS)
{
    /*
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
    */
}

void SceneRender::drawSkybox(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<Shape> shape)
{
    /*
    curS->bind();
    glUniform1i(curS->getUniform("flip"), 0);
    texture1->bind(curS->getUniform("Texture0"));
    */

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

    /*

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
    */
}

// draw map / scene / level / scene
void SceneRender::drawHierMap(shared_ptr<Program> curS, shared_ptr<MatrixStack> model, vector<shared_ptr<Shape>> shape)
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

void SceneRender::drawTextureMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    obj->updateBounds();

    Model->translate(obj->position);
    //Model->translate(obj->position); // move to ground (half of height)
    Model->rotate(obj->angle, obj->rotation);
    Model->scale(obj->scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    obj->updateBounds();

    Model->translate(obj->position);

    Model->rotate(obj->angle, obj->rotation); // FIXME
    Model->scale(obj->scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    obj->updateBounds();

    Model->translate(obj->position + translate);

    Model->rotate(obj->angle + angle, obj->rotation + rotate); // FIXME
    Model->scale(obj->scale + scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawSceneGraph(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
{
    for (auto &child : obj->children)
    {
        Model->pushMatrix();
            Model->loadIdentity();

            child->updateBounds();
            Model->translate(child->position);
            Model->scale(child->scale);
            Model->scale(1.0 / child->shape->largeExtent());

            GLSLUtils::SetMaterial(curS, material);
            GLSLUtils::setModel(curS, Model);
            child->shape->draw(curS);
        Model->popMatrix();
    }
}

void SceneRender::drawPortalFrame(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, bool useMainCamera)
{
    scene->prog->bind();
        if(useMainCamera)
            scene->mainCamera->SetView(scene->prog);
        else
            scene->portalCamera->SetPortalView(scene->texProg, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);    
            
        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        // Draw portal ENTRANCE frame
        sceneRender->drawMesh(scene->prog, scene->ModelPortalSource, scene->portalEntranceDoor, 1);

        // Draw portal EXIT frame
        sceneRender->drawMesh(scene->prog, scene->ModelPortalDestination, scene->portalExitDoor, 2);
    scene->prog->unbind();
}

void SceneRender::drawNonPortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks)
{
    // DRAW MAP
    scene->texProg->bind();
    // set up all the matrices
        scene->portalCamera->SetRecursivePortalView(scene->texProg, scene->tempCamera, scene->ModelPortalSource, scene->ModelPortalDestination);
        glUniformMatrix4fv(scene->texProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->mapGeomNoHier);
    scene->texProg->unbind();

    scene->prog->bind();
        // set up all the matrices
        scene->portalCamera->SetRecursivePortalView(scene->texProg, scene->tempCamera, scene->ModelPortalSource, scene->ModelPortalDestination);
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        scene->skybox->position = glm::vec3(1.0f);
        sceneRender->drawSceneGraph(scene->prog, scene->Model, scene->cube, 1);
        sceneRender->drawMesh(scene->prog, scene->Model, scene->skybox, 0);
    scene->prog->unbind();
}

void SceneRender::drawRecursivePortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, shared_ptr<MatrixStack> viewMat, shared_ptr<MatrixStack> Projection, int maxRecursionLevel, int recursionLevel)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
    glDepthMask(GL_FALSE); // Depth buffer

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST); // enable writing to the stencil buffer
    
    glStencilFunc(GL_NOTEQUAL, recursionLevel, 0xFF);

    glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
    
    glStencilMask(0xFF); // each bit is written to the stencil buffer as is

    // DRAW PORTAL FRAMES
    drawPortalFrame(scene, sceneRender, callbacks, true);
        
    // Calculate view matrix as if the player was already teleported
    //glm::mat4 destView = viewMat * portal.modelMat()
    //    * glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f) * portal.orientation())
    //    * glm::inverse(portal.destination()->modelMat());

    // render inside of portal
    if(recursionLevel == maxRecursionLevel)
    {
        // Setup stencil buffer to draw other objects over the portal
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Color buffer
        glDepthMask(GL_TRUE); // Depth buffer

        glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer

        glEnable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST);
        
        glStencilMask(0x00); // Do not write to stencil buffer
        
        glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

        // REDRAW SCENE IN PORTAL - Redraw scene but with portal view (portal camera)
        drawNonPortals(scene, sceneRender, callbacks);
    }
    else
    {
        // Recursion Case
        drawRecursivePortals(scene, sceneRender, callbacks, viewMat, Projection, maxRecursionLevel, recursionLevel + 1);
    }

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
    glDepthMask(GL_FALSE); // Depth buffer

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); // Enalbe write to stencil buffer

    glStencilFunc(GL_NOTEQUAL, recursionLevel + 1, 0xFF);

    glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

    drawPortalFrame(scene, sceneRender, callbacks, false);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDepthFunc(GL_ALWAYS);

    glClear(GL_DEPTH_BUFFER_BIT);

    // DRAW PORTAL FRAMES
    drawPortalFrame(scene, sceneRender, callbacks, false);

    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glStencilFunc(GL_LEQUAL, 5, 0xFF); // FIXME

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    drawNonPortals(scene, sceneRender, callbacks);
}