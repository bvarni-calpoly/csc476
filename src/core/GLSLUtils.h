#pragma once

#include <memory>
#include "WindowManager.h"
#include "../renderer/MatrixStack.h"
#include "../renderer/Program.h"

class GLSLUtils
{
private:
    GLSLUtils() = delete; // Prevent instantiation FIXME
public:
    static void SetMaterial(std::shared_ptr<Program> curS, int i);
    static void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS);
    static void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> M);
};