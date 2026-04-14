#include <iostream>
#include "AABB.h"

int AABB::intersectsCamera(Camera& cam, const GameObject& obj) // FIXME optimize this, check godot docs
{
    // check each axis for collision
    bool xCollision = (cam.eye.x > obj.shape->min.x) && (cam.eye.x <  obj.shape->max.x);
    bool yCollision = (cam.eye.y > obj.shape->min.y) && (cam.eye.y <  obj.shape->max.y);
    bool zCollision = (cam.eye.z > obj.shape->min.z) && (cam.eye.z <  obj.shape->max.z);

    //std::cout << cam.eye.x << " " << obj.min.x << std::endl;
    //std::cout << xCollision << " " << yCollision << " " << zCollision << std::endl;

    if (xCollision && yCollision && zCollision)
    {
        std::cout << "inside bounding box" << std::endl;
        cam.eye = cam.eye_prev;

        // check if jailed inside object

        return 1; // collision detected
    }

    return 0; // no collision
}

int AABB::intersectsObject(const GameObject& obj1, const GameObject& obj2)
{
    std::cout << "not yet implemented" << std::endl;
    return 0;
}
