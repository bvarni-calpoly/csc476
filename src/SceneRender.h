#pragma once

#include "core/Callbacks.h"
#include "renderer/MatrixStack.h"
#include "renderer/Program.h"
#include "renderer/Shape.h"
#include "world/GameObject.h"
#include "SceneInitializer.h"

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
    void drawPortalMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
    void drawPortalMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, const std::unique_ptr<GameObject> &obj, int material);
    void drawMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
    void drawMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale);
    void drawHierMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
    void drawTextureMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj);
    void drawTextureMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, glm::vec3 translate, float angle, glm::vec3 rotate, glm::vec3 scale);
    void drawTextureHierMesh(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj);
    void drawSceneGraph(std::shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model, std::shared_ptr<GameObject> obj, int material);
    
    void drawPortalFrame(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, bool useMainCamera);
    void drawNonPortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat);
    void drawRecursivePortals(std::shared_ptr<SceneInitializer> scene, std::shared_ptr<SceneRender> sceneRender, std::shared_ptr<Callbacks> callbacks, glm::mat4 viewMat, std::shared_ptr<MatrixStack> Projection, int maxRecursionLevel, int recursionLevel);
};