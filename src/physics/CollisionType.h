#pragma once

#include <glm/glm.hpp>

struct CollisionPlaneResult
{
    bool collided;
    glm::vec3 normal;
    float planeDistance;
};