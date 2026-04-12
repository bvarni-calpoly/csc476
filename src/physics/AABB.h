#pragma once

#include "../world/GameObject.h"
#include "../world/Camera.h"

// https://github.com/godotengine/godot/blob/1aabcb9e9bc7a222a972731523831ec86b77ee20/core/math/aabb.h

class AABB
{
private:
    AABB() = delete; // Prevent instantiation
public:
    static int intersectsCamera(Camera& cam, const GameObject& obj);
    static int intersectsObject(const GameObject& obj1, const GameObject& obj2);
};