#pragma once

#include "../world/GameObject.h"
#include "../world/Camera.h"

class SceneInitializer;

// https://github.com/godotengine/godot/blob/1aabcb9e9bc7a222a972731523831ec86b77ee20/core/math/aabb.h

class AABB
{
private:
    AABB() = delete; // Prevent instantiation
public:
    static int intersectsCamera(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj);
    static int intersectsObject(const GameObject &obj1, const GameObject &obj2);
    static int intersectsCameraPlaneAABB(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj);   // FIXME optimize this, check godot docs
    static int intersectsCameraSinglePlane(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj); // FIXME optimize this, check godot docs
    static int intersectsConvexShape(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj);       // FIXME optimize this, check godot docs
};