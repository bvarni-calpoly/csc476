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
    void drawMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj);
};