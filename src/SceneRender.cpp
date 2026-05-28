#include <iostream>
#include <glad/glad.h>
#include "SceneRender.h"
#include "physics/AABB.h"
#include "core/GLSLUtils.h"
#include "core/Callbacks.h"
#include "world/GameObject.h"
#include "SceneInitializer.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

using namespace std;

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
    // Model->loadIdentity();

    // update matrices
    obj->updateBounds();

    Model->translate(obj->position);
    // Model->translate(obj->position); // move to ground (half of height)
    Model->rotate(obj->angle, obj->rotation);
    Model->scale(obj->scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawTextureMeshNoScale(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj)
{
    Model->pushMatrix();
    // update matrices
    obj->updateBounds();

    // Model->translate(obj->position);
    // Model->rotate(obj->angle, obj->rotation);

    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawTextureHierMesh(shared_ptr<Program> curS, std::shared_ptr<SceneInitializer> scene, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj)
{
    Model->pushMatrix();
    scene->texture1->bind(scene->texProg->getUniform("Texture0"));

    for (const auto &part : obj->children)
    {
        if (part->portal->portalID == 0)
        {
            Model->pushMatrix();
            if (part->objName.find("white") != string::npos)
                scene->textureWhiteTile->bind(scene->texProg->getUniform("Texture0"));
            else if (part->objName.find("black") != string::npos)
                scene->textureBlackTile->bind(scene->texProg->getUniform("Texture0"));
            else if (part->objName.find("blackstripe") != string::npos)
                scene->textureBlackStripeTile->bind(scene->texProg->getUniform("Texture0"));
            else
                scene->textureBlackStripeTile->bind(scene->texProg->getUniform("Texture0"));

            if (part->objName.find("collide") != string::npos)
            {
                if (part->collided % 2 == 1)
                    scene->textureWhiteTile->bind(scene->texProg->getUniform("Texture0"));
                else
                    scene->textureBlackTile->bind(scene->texProg->getUniform("Texture0"));
            }

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
    // Model->loadIdentity();

    // update matrices
    // obj->updateBounds();

    Model->translate(obj->position);
    Model->rotate(obj->angle, obj->rotation); // FIXME
    Model->translate(-obj->shape->center);    // FIXME

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawPortalMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, GameObject *obj, int material)
{
    Model->pushMatrix();
    // Model->loadIdentity();

    // update matrices
    // obj->updateBounds();

    Model->translate(obj->position);
    Model->rotate(obj->angle, obj->rotation); // FIXME
    Model->translate(-obj->shape->center);    // FIXME

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawPortalMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> &Model, const unique_ptr<GameObject> &obj, int material)
{
    Model->pushMatrix();
    Model->loadIdentity();

    // update matrices
    // obj->updateBounds();

    // move to position
    // Model->translate(obj->position);

    // rotate according to surface normal
    // Model->multMatrix(obj->rotationMat);

    // Model->scale(1 / 20.0f);

    // // normalize
    // Model->scale(1.0 / obj->shape->largeExtent());

    // Model->translate(-obj->shape->center); // center in world

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
{
    Model->pushMatrix();
    // Model->loadIdentity();

    // update matrices
    obj->updateBounds();

    Model->translate(obj->position);
    Model->rotate(obj->angle, obj->rotation); // FIXME
    Model->scale(obj->scale);
    Model->scale(1.0 / obj->shape->largeExtent()); // normalize
    Model->translate(-obj->shape->center);         // FIXME

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}

void SceneRender::drawMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale)
{
    Model->pushMatrix();
    // Model->loadIdentity();

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
    // Model->loadIdentity();

    // update matrices
    // obj->updateBounds();

    // Model->translate(obj->position);
    // Model->rotate(obj->angle, obj->rotation);
    // Model->scale(obj->scale);
    // Model->scale(1.0 / obj->shape->largeExtent()); // normalize

    for (const auto &part : obj->children)
    {
        if (part->portal->portalID > 0)
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

void SceneRender::drawPortalFrame(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, glm::mat4 projMat)
{
    scene->prog->bind();
    scene->mainCamera->SetView(scene->prog);
    scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);

    // set up all the matrices
    glUniformMatrix4fv(scene->prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(projMat));
    glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

    // draw portals from the map obj
    for (const auto &child : scene->mapGeom->children)
    {
        // Draw portal frame
        if (child->portal->portalID > 0)
        {
            sceneRender->drawPortalMesh(scene->prog, scene->Model, child, 0);
            // break; // FIXME investigate why the other quad draws without this
        }
    }
    scene->prog->unbind();
}

void SceneRender::drawPortalFrame(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, GameObject *portal, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, glm::mat4 projMat)
{
    scene->prog->bind();
    scene->mainCamera->SetView(scene->prog);
    scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);

    // set up all the matrices
    glUniformMatrix4fv(scene->prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(projMat));
    glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

    sceneRender->drawPortalMesh(scene->prog, scene->Model, portal, portal->portal->portalID);
    // // draw portals from the map obj
    // for (const auto &child : scene->mapGeom->children)
    // {
    //     if (child->portal->portalID != 0)
    //     {
    //         // Draw portal ENTRANCE frame
    //         if(child->portal->portalID == portal->portal->portalID)
    //             sceneRender->drawPortalMesh(scene->prog, scene->Model, child, child->portal->portalID);

    //         //break; // FIXME investigate why the other quad draws without this
    //     }
    // }
    scene->prog->unbind();
}

void SceneRender::drawNonPortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 destView, glm::mat4 projMat)
{
    // Draw map
    scene->texProg->bind();
    scene->texture1->bind(scene->texProg->getUniform("Texture1"));

    // set up all the matrices
    glUniformMatrix4fv(scene->texProg->getUniform("V"), 1, GL_FALSE, glm::value_ptr(destView));
    glUniformMatrix4fv(scene->texProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(projMat));

    // --- Send light data ---
    LightBlockUBO lightData;

    PointLightUBO playerLight;
    playerLight.position = glm::vec4(scene->mainCamera->eye, 0.0f);
    playerLight.color = glm::vec4(1.0f);
    playerLight.intensity = glm::vec4(10.1f);

    PointLightUBO projectileLight;
    projectileLight.position = glm::vec4(scene->projectile->position, 0.0f);
    projectileLight.color = glm::vec4(1.0f, 1.0f, 0.5f, 0.0f);
    projectileLight.intensity = glm::vec4(1.0f);

    PointLightUBO redLight;
    redLight.position = glm::vec4(1000.0f, 100.0f, 1000.0f, 0.0f);
    redLight.color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    redLight.intensity = glm::vec4(1.0f);

    PointLightUBO greenLight;
    greenLight.position = glm::vec4(-1000.0f, 100.0f, 1000.0f, 0.0f);
    greenLight.color = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    greenLight.intensity = glm::vec4(1.0f);

    PointLightUBO blueLight;
    blueLight.position = glm::vec4(-1000.0f, 100.0f, -1000.0f, 0.0f);
    blueLight.color = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    blueLight.intensity = glm::vec4(1.0f);

    PointLightUBO pinkLight;
    pinkLight.position = glm::vec4(1000.0f, 100.0f, -1000.0f, 0.0f);
    pinkLight.color = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
    pinkLight.intensity = glm::vec4(1.0f);

    // Follows a circular path
    PointLightUBO rainbowLight;
    float time = glfwGetTime();
    float speed = 1.0f;
    float radius = 1000.0f;

    glm::vec3 rainbowColor = glm::vec3( // 2pi/3 = 6.28/3 = 2.093
        glm::sin((time + 0.0f) + 1.0f) / 2.0f,
        glm::sin((time + 2.093f) + 1.0f) / 2.0f,
        glm::sin((time + 4.18f) + 1.0f) / 2.0f);
    rainbowLight.position = glm::vec4(glm::sin(time * speed) * radius, 10.0f, glm::cos(time * speed) * radius, 1.0f);
    rainbowLight.color = glm::vec4(rainbowColor, 0.0f);
    rainbowLight.intensity = glm::vec4(5.0f);

    lightData.lights[0] = playerLight;
    lightData.lights[1] = projectileLight;
    lightData.lights[2] = redLight;
    lightData.lights[3] = greenLight;
    lightData.lights[4] = blueLight;
    lightData.lights[5] = pinkLight;
    lightData.lights[6] = rainbowLight;
    lightData.numActiveLights = glm::ivec4(7);

    // fixme
    glBindBuffer(GL_UNIFORM_BUFFER, scene->uboLightBlock);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightBlockUBO), &lightData);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // end of fixme

    glUniform1f(scene->texProg->getUniform("glowIntensity"), 0.0f); // default glow of none
    glUniform1i(scene->texProg->getUniform("flip"), 1);

    // Draw Texture Cube
    scene->textureTile->bind(scene->texProg->getUniform("Texture0"));
    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->texture_cube);

    // Draw map
    sceneRender->drawTextureHierMesh(scene->texProg, scene, scene->Model, scene->mapGeom);
    scene->texture1->bind(scene->texProg->getUniform("Texture0")); // reset texture

    // Draw projectile
    glUniform1f(scene->texProg->getUniform("glowIntensity"), 2.0f); // add glow
    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->projectile);

    // Draw pawn
    scene->textureWhiteTile->bind(scene->texProg->getUniform("Texture0"));
    // if (scene->pawn->collided % 2 == 1)
    //     scene->textureWhiteTile->bind(scene->texProg->getUniform("Texture0"));
    // else
    //     scene->textureBlackTile->bind(scene->texProg->getUniform("Texture0"));
    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->pawn);

    glUniform1f(scene->texProg->getUniform("glowIntensity"), 0.0f); // default glow of none
    scene->texture0->bind(scene->texProg->getUniform("Texture0"));

    // Draw skybox
    glUniform1i(scene->texProg->getUniform("flip"), 0);
    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->skybox);
    glUniform1i(scene->texProg->getUniform("flip"), 1);

    // Send portal normal and cube position to frag shader
    scene->texture1->bind(scene->texProg->getUniform("Texture0"));
    glUniform3fv(scene->texProg->getUniform("portalNormal"), 1, glm::value_ptr(glm::vec3(0, 0, 1))); // FIXME hardcoded
    glUniform3fv(scene->texProg->getUniform("portalPos"), 1, glm::value_ptr(glm::vec3(0.0f)));       // FIXME hardcoded
    glUniform1i(scene->texProg->getUniform("useSlicing"), 1);
    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->portalcube);
    glUniform1i(scene->texProg->getUniform("useSlicing"), 0);

    sceneRender->drawTextureMeshNoScale(scene->texProg, scene->Model, scene->testcube);
    scene->texProg->unbind();

    scene->prog->bind();
    // set up all the matrices
    // scene->portalCamera->SetPortalView(scene->prog, scene->mainCamera, scene->ModelPortalSource, scene->ModelPortalDestination);
    glUniformMatrix4fv(scene->prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(destView));
    glUniformMatrix4fv(scene->prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(projMat));
    glUniform3fv(scene->prog->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));

    // update player position - FIXME, USE NEAR CLIPPING PLANE
    scene->player->position = scene->mainCamera->eye + (scene->mainCamera->forward * -15.0f);

    scene->cube->position = glm::vec3(1.0f);
    sceneRender->drawSceneGraph(scene->prog, scene->Model, scene->cube, 1);
    sceneRender->drawMesh(scene->prog, scene->Model, scene->player, 1);
    scene->prog->unbind();
}

glm::mat4 const SceneRender::clippedProjMat(GameObject &portal, glm::mat4 const &viewMat, glm::mat4 const &projMat)
{
    // float dist = glm::length(d_position);
    // glm::vec4 clipPlane(d_orientation * glm::vec3(0.0f, 0.0f, -1.0f), dist);
    float dist = glm::length(portal.position);
    glm::vec4 clipPlane(portal.normals[0], dist);
    clipPlane = glm::inverse(glm::transpose(viewMat)) * clipPlane;

    if (clipPlane.w > 0.0f)
        return projMat;

    glm::vec4 q = glm::inverse(projMat) * glm::vec4(
                                              glm::sign(clipPlane.x),
                                              glm::sign(clipPlane.y),
                                              1.0f,
                                              1.0f);

    glm::vec4 c = clipPlane * (2.0f / (glm::dot(clipPlane, q)));

    glm::mat4 newProj = projMat;
    // third row = clip plane - fourth row
    newProj = glm::row(newProj, 2, c - glm::row(newProj, 3));

    return newProj;
}

/*
// https://github.com/ThomasRinsma/opengl-game-test/blob/8363bbf/src/scene.cc#L81
void SceneRender::drawRecursivePortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, shared_ptr<MatrixStack> Projection, int maxRecursionLevel, int recursionLevel)
{
    // for (auto &pair : scene->portals)
    for (auto &portal : scene->portals)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
        glDepthMask(GL_FALSE); // Depth buffer

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST); // enable writing to the stencil buffer

        glStencilFunc(GL_NOTEQUAL, recursionLevel, 0xFF);

        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);

        glStencilMask(0xFF); // Enable writing into all stencil bits

        // Draw portal frames into setencil buffer
        drawPortalFrame(scene, sceneRender, callbacks, viewMat);

        // Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
        //TD = TB^-1 * R * TA * TC
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // R, rotate 180 degrees
        glm::mat4 portalA =  glm::translate(glm::mat4(1.0f), portal->position);                 // TA
        glm::mat4 portalB =  glm::translate(glm::mat4(1.0f), portal->destination->position);    // TB
        // 1. inverse(PortalB) - Move from world space -> destination local space
        // 2. Rotation		   - (optional) flip orientation 180 degrees
        // 3. Portal A		   - Move from rotated destination local space -> source world space
        // 4. mainView		   - Move from source world space -> camera space

        glm::mat4 destView = viewMat
            * portalB
            //* rotation
            * glm::inverse(portalA);

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
            //drawNonPortals(scene, sceneRender, callbacks, clippedProjMat(*portal, destView, Projection->topMatrix()));
        }
        else
        {
            // Recursion Case
            drawRecursivePortals(scene, sceneRender, callbacks, destView, Projection, maxRecursionLevel, recursionLevel + 1);
            //drawRecursivePortals(scene, sceneRender, callbacks, clippedProjMat(*portal, destView, Projection->topMatrix()), Projection, maxRecursionLevel, recursionLevel + 1);
        }

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
        glDepthMask(GL_FALSE); // Depth buffer

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF); // Enalbe write to stencil buffer

        glStencilFunc(GL_NOTEQUAL, recursionLevel + 1, 0xFF);

        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        drawPortalFrame(scene, sceneRender, portal, callbacks, viewMat);
    }

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDepthFunc(GL_ALWAYS);

    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw portals into depth buffer
    drawPortalFrame(scene, sceneRender, callbacks, viewMat);

    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glStencilFunc(GL_LEQUAL, recursionLevel, 0xFF);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    drawNonPortals(scene, sceneRender, callbacks, viewMat);
}
*/

/*
// https://github.com/ThomasRinsma/opengl-game-test/blob/8363bbf/src/scene.cc#L81
void SceneRender::drawRecursivePortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, shared_ptr<MatrixStack> Projection, int maxRecursionLevel, int recursionLevel)
{
    // for (auto &pair : scene->portals)
    for (auto &portal : scene->portals)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
        glDepthMask(GL_FALSE); // Depth buffer

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_STENCIL_TEST); // enable writing to the stencil buffer

        glStencilFunc(GL_EQUAL, recursionLevel, 0xFF);

        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glStencilMask(0xFF); // Enable writing into all stencil bits

        // Draw portal frames into setencil buffer
        drawPortalFrame(scene, sceneRender, callbacks, viewMat);

        // Setup stencil buffer to draw other objects over the portal
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Color buffer
        glDepthMask(GL_TRUE); // Depth buffer

        glEnable(GL_STENCIL_TEST);

        glStencilMask(0x00); // Do not write to stencil buffer

        glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);

        //glDepthRange(1, 1);

        //glDepthRange(0, 1);
        glDepthFunc(GL_LESS);

        //glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer


        // Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
        //TD = TB^-1 * R * TA * TC
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // R, rotate 180 degrees
        glm::mat4 portalA =  glm::translate(glm::mat4(1.0f), portal->position);                 // TA
        glm::mat4 portalB =  glm::translate(glm::mat4(1.0f), portal->destination->position);    // TB
        // 1. inverse(PortalB) - Move from world space -> destination local space
        // 2. Rotation		   - (optional) flip orientation 180 degrees
        // 3. Portal A		   - Move from rotated destination local space -> source world space
        // 4. mainView		   - Move from source world space -> camera space

        glm::mat4 destView = viewMat
            * portalB
            //* rotation
            * glm::inverse(portalA);

        // render inside of portal
        if(recursionLevel == maxRecursionLevel)
        {

            // REDRAW SCENE IN PORTAL - Redraw scene but with portal view (portal camera)
            drawNonPortals(scene, sceneRender, callbacks, destView);
            //drawNonPortals(scene, sceneRender, callbacks, clippedProjMat(*portal, destView, Projection->topMatrix()));
        }
        else
        {
            // Recursion Case
            drawRecursivePortals(scene, sceneRender, callbacks, destView, Projection, maxRecursionLevel, recursionLevel + 1);
            //drawRecursivePortals(scene, sceneRender, callbacks, clippedProjMat(*portal, destView, Projection->topMatrix()), Projection, maxRecursionLevel, recursionLevel + 1);
        }

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer

        glDepthMask(GL_TRUE); // Depth buffer
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);

        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

        glStencilMask(0xFF); // Enalbe write to stencil buffer

        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

        drawPortalFrame(scene, sceneRender, portal, callbacks, viewMat);

        glDepthFunc(GL_LESS);
    }

    // glDisable(GL_STENCIL_TEST);
    // glStencilMask(0x00);

    // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // glDepthMask(GL_TRUE);


    // glClear(GL_DEPTH_BUFFER_BIT);

    // Draw portals into depth buffer
    //drawPortalFrame(scene, sceneRender, callbacks, viewMat);
    //glEnable(GL_STENCIL_TEST);

    glStencilFunc(GL_LEQUAL, recursionLevel, 0xFF);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0x00);

    glEnable(GL_DEPTH_TEST);

    drawNonPortals(scene, sceneRender, callbacks, viewMat);
}
*/

// https://github.com/ThomasRinsma/opengl-game-test/blob/8363bbf/src/scene.cc#L81
void SceneRender::drawPortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, glm::mat4 projMat)
{
    for (auto &portal : scene->portals)
    {
        // Disable color and depth buffers
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color buffer
        glDepthMask(GL_FALSE);                               // Depth buffer
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST); // Enable writing to the stencil buffer

        // Stencil fails on empty pixels, causing GL_INCR to set pixel to 1
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
        glStencilMask(0xFF); // Enable writing into all stencil bits

        // Draw portal frames into stencil buffer
        drawPortalFrame(scene, sceneRender, callbacks, viewMat, projMat);

        // --- Setup drawing objects inside the portal frame ---
        // Enable color and depth buffers
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Color buffer
        glDepthMask(GL_TRUE);                            // Depth buffer
        glEnable(GL_DEPTH_TEST);

        // THIS LINE WILL BREAK SOME PORTALS
        // glClear(GL_DEPTH_BUFFER_BIT); // FIXME, THIS WILL BREAK SOME PORTALS -Clear depth buffer

        // Setup depth tests and stencil, only draw pixels where stencil is 1
        glStencilMask(0x00); // Lock stencil buffer
        glStencilFunc(GL_EQUAL, 1, 0xFF);

        // Generate the virtual camera’s view matrix using the view frustum clipping method, check main file sources for more information
        // TD = TB^-1 * R * TA * TC
        // 1. inverse(PortalB) - Move from world space -> destination local space
        // 2. Rotation		   - (optional) flip orientation 180 degrees
        // 3. Portal A		   - Move from rotated destination local space -> source world space
        // 4. mainView		   - Move from source world space -> camera space

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));                                                // R, rotate 180 degrees
        glm::mat4 portalA = glm::translate(glm::mat4(1.0f), portal.second->position) * portal.second->rotationMat;                                           // TA
        glm::mat4 portalB = glm::translate(glm::mat4(1.0f), portal.second->portal->destination->position) * portal.second->portal->destination->rotationMat; // TB
        glm::mat4 destView = viewMat * portalB * rotation * glm::inverse(portalA);
        // glm::mat4 destView = viewMat * portalB * glm::inverse(portalA);

        // proj mat
        // glm::mat4 proj = sceneRender->clippedProjMat(*portal, destView  , projMat);
        glm::mat4 proj = projMat;

        // Redraw scene but with portal view (portal camera)
        drawNonPortals(scene, sceneRender, callbacks, destView, proj);
        // drawNonPortals(scene, sceneRender, callbacks, clippedProjMat(*portal, destView, Projection->topMatrix()));
    }

    // --- Draw remaining scene outside of the portals ---
    // Stop writing to the stencil buffer
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    // Change write status to buffers
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable writing to the color buffer
    glDepthMask(GL_TRUE);                                // Enable writing to the depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable depth test and clear depth buffer
    glDepthFunc(GL_ALWAYS);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    // Draw portals into depth buffer
    drawPortalFrame(scene, sceneRender, callbacks, viewMat, projMat);

    // Enable color and depth buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // Draw whole scene with main camera
    drawNonPortals(scene, sceneRender, callbacks, viewMat, projMat);
}

void SceneRender::drawTool(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, float deltaTime)
{
    scene->texProg->bind();
    // set up all the matrices
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(scene->texProg->getUniform("V"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(scene->texProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->Projection->topMatrix()));
    // glUniform3fv(scene->texProg->getUniform("lightPos"), 1, glm::value_ptr(callbacks->lightTrans));
    scene->texture1->bind(scene->texProg->getUniform("Texture0"));

    scene->tool->position = glm::vec3(0.45f, -0.45f, -0.75f) + sin(scene->mainCamera->eye / 20.0f) / 10.0f;
    scene->tool->rotation = glm::vec3(1, 0, 0);
    scene->tool->angle = scene->playerCamera->weaponAngle;

    if (scene->playerCamera->reloading)
    {
        scene->playerCamera->reloadAnimation(scene, deltaTime);
    }

    sceneRender->drawTextureMesh(scene->texProg, scene->Model, scene->tool);

    glUniform1f(scene->texProg->getUniform("glowIntensity"), 0.0f); // reset glow
    scene->texProg->unbind();
}