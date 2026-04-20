#include "SceneRender.h"
#include "core/GLSLUtils.h"
#include "world/GameObject.h"

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

void SceneRender::drawMesh(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<GameObject> obj, int material)
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

    GLSLUtils::SetMaterial(curS, material);
    GLSLUtils::setModel(curS, Model);
    obj->shape->draw(curS);
    Model->popMatrix();
}