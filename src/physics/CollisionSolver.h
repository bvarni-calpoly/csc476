#pragma once

#include "../world/GameObject.h"
#include "../world/Camera.h"
#include "CollisionType.h"

class SceneInitializer;

// https://github.com/godotengine/godot/blob/1aabcb9e9bc7a222a972731523831ec86b77ee20/core/math/aabb.h

class CollisionSolver
{
private:
    CollisionSolver() = delete; // Prevent instantiation
public:
    static int testPlayerVsAABB(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj);
    static int testAABBvsAABB(const GameObject &obj1, const GameObject &obj2);
    static float solveRaycastVsAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const GameObject &obj2);
    static int testPlayerVsPlane(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj);                    // FIXME optimize this, check godot docs
    static int testObjVsPlane(const GameObject &obj, const GameObject &objPortal);                                    // FIXME optimize this, check godot docs
    static CollisionPlaneResult solvePlayerVsConvex(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj); // FIXME optimize this, check godot docs
    static CollisionPlaneResult solvePointVsConvex(const glm::vec3 point, const GameObject &obj);                     // FIXME optimize this, check godot docs
};