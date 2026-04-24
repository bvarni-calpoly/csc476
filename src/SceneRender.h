#pragma once

#include "renderer/MatrixStack.h"
#include "renderer/Program.h"
#include "renderer/Shape.h"
#include "world/GameObject.h"

class SceneRender
{
private:
    /* data */
public:
    SceneRender(/* args */);
    ~SceneRender();

    void drawGround(std::shared_ptr<Program> curS);
    void drawSkybox(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<Shape> shape);
    void drawHierMap(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> model, std::vector<std::shared_ptr<Shape>> shape);
    void drawMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
    void drawMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale);
    void drawTextureMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj);
    void drawTextureMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale);
    void drawSceneGraph(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
};