#include <iostream>
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

    // draw the skybox
    Model->pushMatrix();
    Model->loadIdentity();

    float halfHeight = (shape->max.y - shape->min.y) / 2.0f;

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

void SceneRender::drawTextureHierMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    //obj->updateBounds();

    Model->translate(obj->position);
    Model->rotate(obj->angle, obj->rotation);
    Model->scale(1.0f / 20.0f);
    //Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    for(const auto &part : obj->children)
    {
        if(part->portalID == 0)
        {
            Model->pushMatrix();
                // move to position
                //Model->translate(part->position);

                // rotate according to surface normal
                //Model->multMatrix(obj->rotationMat);

                // normalize
                // Model->scale(glm::vec3(100.0f));
                // Model->scale(1.0 / part->shape->largeExtent());
                // Model->translate(-part->shape->center); // center in world
            
                GLSLUtils::setModel(curS, Model);
                part->shape->draw(curS);
            Model->popMatrix();
        }
    }
    Model->popMatrix();
}

void SceneRender::drawPortalMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    //obj->updateBounds();

    Model->translate(obj->position);
    Model->rotate(obj->angle, obj->rotation); // FIXME
    Model->translate(-obj->shape->center); // FIXME
    
    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawPortalMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> &Model, const unique_ptr<GameObject> &obj, int material)
{
    Model->pushMatrix();
        //Model->loadIdentity();

        // update matrices
        //obj->updateBounds();

        // move to position
        //Model->translate(obj->position);

        // // rotate according to surface normal
        // Model->multMatrix(obj->rotationMat);

        Model->scale(1 / 20.0f);

        // // normalize
        //Model->scale(1.0 / obj->shape->largeExtent());
        
        //Model->translate(-obj->shape->center); // center in world

        GLSLUtils::SetMaterial(curS, material);
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
    Model->translate(-obj->shape->center); // FIXME
    
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

    Model->translate(obj->position + translate);

    Model->rotate(obj->angle + angle, obj->rotation + rotate); // FIXME
    Model->scale(obj->scale + scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawHierMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
{
    Model->pushMatrix();
    //Model->loadIdentity();

    // update matrices
    //obj->updateBounds();

    //Model->translate(obj->position);
    //Model->rotate(obj->angle, obj->rotation);
    //Model->scale(obj->scale);
    //Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    for(const auto &part : obj->children)
    {
        if(part->portalID > 0)
        {
            break;
        }

        GLSLUtils::SetMaterial(curS, material);
        GLSLUtils::setModel(curS, Model);
        part->shape->draw(curS);
    }
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

void SceneRender::drawPortalFrame(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, bool useMainCamera)
{
    scene->prog->bind();
        if(useMainCamera)
            scene->mainCamera->SetView(scene->prog);
        else
            glUniformMatrix4fv(scene->prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(viewMat));
            //scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);    
            
        // set up all the matrices
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        // from obj
        // FIXME
        for (const auto &child : scene->mapGeom->children)
        {
            if (child->portalID != 0)
            {
                // Draw portal ENTRANCE frame
                if(child->objName.find("entrance") != string::npos)
                    sceneRender->drawPortalMesh(scene->prog, scene->ModelPortalSource, child, child->portalID);

                // Draw portal EXIT frame
                if(child->objName.find("exit") != string::npos)
                    sceneRender->drawPortalMesh(scene->prog, scene->ModelPortalDestination, child, child->portalID);
                
                //break; // FIXME investigate why the other quad draws without this
            }
        }

        // hardcoded
        // Draw portal ENTRANCE frame
        //sceneRender->drawPortalMesh(scene->prog, scene->ModelPortalSource, scene->portalEntranceDoor, 1);

        // Draw portal EXIT frame
        //sceneRender->drawPortalMesh(scene->prog, scene->ModelPortalDestination, scene->portalExitDoor, 2);
    scene->prog->unbind();
}

void SceneRender::drawNonPortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 destView)
{
    // Draw map
    scene->texProg->bind();
    // set up all the matrices
        //scene->portalCamera->SetPortalView(scene->texProg, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);
	    glUniformMatrix4fv(scene->texProg->getUniform("V"), 1, GL_FALSE, glm::value_ptr(destView));
        glUniformMatrix4fv(scene->texProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        sceneRender->drawTextureHierMesh(scene->texProg, scene->Model, scene->mapGeom);
        sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->skybox);
    scene->texProg->unbind();

    scene->prog->bind();
        // set up all the matrices
        //scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);
	    glUniformMatrix4fv(scene->prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(destView));
        glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
        glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

        // update player position
        scene->player->position = scene->mainCamera->eye + glm::vec3(0.0f, 1.0f, 0.0f);

        scene->cube->position = glm::vec3(1.0f);
        //sceneRender->drawSceneGraph(scene->prog, scene->Model, scene->cube, 1);
        sceneRender->drawMesh(scene->prog, scene->Model, scene->player, 1);
    scene->prog->unbind();

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
    */
}

// https://github.com/ThomasRinsma/opengl-game-test/blob/8363bbf/src/scene.cc#L81
void SceneRender::drawRecursivePortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, shared_ptr<MatrixStack> Projection, int maxRecursionLevel, int recursionLevel)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
    glDepthMask(GL_FALSE); // Depth buffer

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST); // enable writing to the stencil buffer
    
    glStencilFunc(GL_NOTEQUAL, recursionLevel, 0xFF);

    glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
    
    glStencilMask(0xFF); // each bit is written to the stencil buffer as is

    // Get portals from obj file
    GameObject* portal1 = nullptr;
    GameObject* portal2 = nullptr;

    for(auto &child : scene->mapGeom->children)
    {
        if(child->portalID == 1) portal1 = child.get();
        if(child->portalID == 2) portal2 = child.get();
    }

    // DRAW PORTAL FRAMES
    drawPortalFrame(scene, sceneRender, callbacks, viewMat, true);
        
    // shared_ptr<MatrixStack> destView = viewMat->topMatrix() * scene->ModelPortalSource->topMatrix()
    //    * glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f) * scene->ModelPortalSource.orientation())
    //    * glm::inverse(scene->ModelPortalDestination->topMatrix());

    // Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
	//TD = TB^-1 * R * TA * TC
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)); // R, rotate 180 degrees
	glm::mat4 portalA = scene->ModelPortalSource->topMatrix();	    // TA
	glm::mat4 portalB = scene->ModelPortalDestination->topMatrix();	// TB
	// 1. inverse(PortalB) - Move from world space -> destination local space
	// 2. Rotation		   - (optional) flip orientation 180 degrees
	// 3. Portal A		   - Move from rotated destination local space -> source world space
	// 4. mainView		   - Move from source world space -> camera space

    glm::mat4 destView = viewMat
        * portalA
        * rotation
        * glm::inverse(portalB);

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
        drawNonPortals(scene, sceneRender, callbacks, destView);
    }
    else
    {
        // Recursion Case
        drawRecursivePortals(scene, sceneRender, callbacks, destView, Projection, maxRecursionLevel, recursionLevel + 1);
    }

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
    glDepthMask(GL_FALSE); // Depth buffer

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); // Enalbe write to stencil buffer

    glStencilFunc(GL_NOTEQUAL, recursionLevel + 1, 0xFF);

    glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

    drawPortalFrame(scene, sceneRender, callbacks, viewMat, false);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDepthFunc(GL_ALWAYS);

    glClear(GL_DEPTH_BUFFER_BIT);

    // DRAW PORTAL FRAMES
    drawPortalFrame(scene, sceneRender, callbacks, viewMat, false);

    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glStencilFunc(GL_LEQUAL, recursionLevel, 0xFF);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    drawNonPortals(scene, sceneRender, callbacks, viewMat);
}