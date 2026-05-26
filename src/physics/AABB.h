#pragma once

#include "../world/GameObject.h"
#include "../world/Camera.h"

// https://github.com/godotengine/godot/blob/1aabcb9e9bc7a222a972731523831ec86b77ee20/core/math/aabb.h

class AABB
{
private:
    AABB() = delete; // Prevent instantiation
public:
    static int intersectsCamera(Camera &cam, const GameObject &obj);
    static int intersectsObject(const GameObject &obj1, const GameObject &obj2);
    static int intersectsCameraPlaneAABB(Camera &cam, const GameObject &obj);   // FIXME optimize this, check godot docs
    static int intersectsCameraSinglePlane(Camera &cam, const GameObject &obj); // FIXME optimize this, check godot docs
    static int intersectsConvexShape(Camera &cam, const GameObject &obj);       // FIXME optimize this, check godot docs
};