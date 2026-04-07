#include "GLSLUtils.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

// helper function to pass material data to the GPU
void GLSLUtils::SetMaterial(shared_ptr<Program> curS, int i)
{
    switch (i)
    {
    case 0: // purple
        glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
        glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
        glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
        glUniform1f(curS->getUniform("MatShine"), 120.0);
        break;
    case 1:
        // https://learnopengl.com/Lighting/Materials
        glUniform3f(curS->getUniform("MatAmb"), sin(glfwGetTime() * 2.0f) / 4 + 0.52, sin(glfwGetTime() * 0.7f) / 4 + 0.25, sin(glfwGetTime() * 1.3f) / 4 + 0.25);
        glUniform3f(curS->getUniform("MatDif"), sin(glfwGetTime() / 4.0f) / 4 + 0.52, sin(glfwGetTime() / 4.0f) / 4 + 0.25, sin(glfwGetTime() / 4.0f) / 4 + 0.25);
        glUniform3f(curS->getUniform("MatSpec"), sin(glfwGetTime() / 4.0f) / 4 + 0.52, sin(glfwGetTime() / 4.0f) / 4 + 0.25, sin(glfwGetTime() / 4.0f) / 4 + 0.25);
        glUniform1f(curS->getUniform("MatShine"), 150.0);
        break;
    }
}

/* helper function to set model transforms */
void GLSLUtils::SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS)
{
    mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
    mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
    mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
    mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
    mat4 ctm = Trans * RotX * RotY * ScaleS;
    glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void GLSLUtils::setModel(shared_ptr<Program> prog, shared_ptr<MatrixStack> M)
{
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}