#include <iostream>
#include "AABB.h"
#include "../SceneInitializer.h"

int AABB::intersectsCamera(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);

    // adjust for player height
    float camHeight = cam.eye.y - player.playerHeight;
    // check each axis for collision
    bool xCollision = (cam.eye.x > obj.min.x) && (cam.eye.x < obj.max.x);
    bool yCollision = (cam.eye.y > obj.min.y) && (camHeight < obj.max.y);
    bool zCollision = (cam.eye.z > obj.min.z) && (cam.eye.z < obj.max.z);

    if (xCollision && yCollision && zCollision)
    {
        // std::cout << "inside bounding box" << std::endl;
        // cam.eye = cam.eye_prev;
        cam.eye.y = obj.max.y + player.playerHeight;
        player.velocity.y = 0;
        player.airborne = false;

        // check if jailed inside object

        return 1; // collision detected
    }

    return 0; // no collision
}

// FIXME THIS MOVE TO ANOTHER CLASS
int AABB::intersectsCameraPlaneAABB(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);

    // adjust for player height
    float camHeight = cam.eye.y - player.playerHeight;

    // check each axis for collisionS
    bool xCollision = (cam.eye.x >= (obj.min.x - 0.5f)) && (cam.eye.x <= (obj.max.x + 0.5f));
    bool yCollision = (cam.eye.y >= (obj.min.y - 0.5f)) && (player.playerHeight <= (obj.max.y + 0.5f));
    bool zCollision = (cam.eye.z >= (obj.min.z - 0.5f)) && (cam.eye.z <= (obj.max.z + 0.5f));

    // std::cout << xCollision << yCollision << zCollision << std::endl;

    if (xCollision && yCollision && zCollision)
    {
        std::cout << "inside bounding plane" << std::endl;
        // cam.eye = cam.eye_prev;
        // cam.eye = glm::vec3(0, 10, 0);
        // cam.velocity.y = 0;
        // cam.airborne = false;

        // check if jailed inside object

        return 1; // collision detected
    }

    return 0; // no collision
}

int AABB::intersectsObject(const GameObject &obj1, const GameObject &obj2)
{
    // check each axis for collision
    bool xCollision = (obj1.max.x >= obj2.min.x) && (obj1.min.x <= obj2.max.x);
    bool yCollision = (obj1.max.y >= obj2.min.y) && (obj1.min.y <= obj2.max.y);
    bool zCollision = (obj1.max.z >= obj2.min.z) && (obj1.min.z <= obj2.max.z);

    if (xCollision && yCollision && zCollision)
        return 1; // collision detected

    return 0; // no collision
}

// FIXME ADD THIS TO ANOTHER CLASS
int AABB::intersectsCameraSinglePlane(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);

    // vertex normal
    glm::vec3 normal = glm::normalize(obj.normals[0]);

    // --- distance = (N * P) + d ---
    // Calculate position of plane relative to normal
    float d = -glm::dot(normal, obj.position);

    // adjust for player height
    glm::vec3 camHeight = cam.eye - glm::vec3(0, player.playerHeight, 0);

    // Distance from camera to plane
    float distance = glm::dot(normal, camHeight) + d;

    // check if on other side of plane
    float threshold = 10.0f;
    if (distance <= 0.0f && distance >= -threshold)
    {
        // offset position to be on correct side of plane
        // glm::vec3 correction = normal * glm::abs(distance);
        // cam.eye += correction;
        // glm::vec3 correction = normal * distance;
        // cam.eye -= correction;

        // // push player out in direction of normal
        // float velocityAlongNormal = glm::dot(cam.velocity, normal);
        // if (velocityAlongNormal < 0.0f)
        // {
        //     cam.velocity -= normal * velocityAlongNormal;
        // }

        // cam.airborne = false;

        return 1; // collision detected
    }

    return 0; // no collision
}

/**
 * https://en.wikipedia.org/wiki/Convex_hull
 * Tests every plane on a convex shape to see if point is inside the shape
 *
 * If the player is inside the convex shape, distance is negative
 * If the player is outside the convex shape, distance is positive
 */
int AABB::intersectsConvexShape(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);

    // adjust for player height
    glm::vec3 camHeight = cam.eye - glm::vec3(0, player.playerHeight, 0);

    // push dir for correcting play position (if collision is detected)
    glm::vec3 pushDir = glm::vec3(0.0f);
    float closestPlaneDistance = -std::numeric_limits<float>::max();

    // check collision against every plane
    for (const auto &currPlane : obj.planes)
    {
        // flip normal
        glm::vec3 normal = glm::normalize(currPlane.normal);

        // --- distance = (N * P) + d ---
        // Calculate position of plane relative to normal
        // float d = -glm::dot(normal, obj.position);
        float d = -glm::dot(normal, currPlane.point);

        // Distance from camera to plane
        float currDistance = glm::dot(normal, camHeight) + d;

        // Check if outside the plane
        if (currDistance > 0.0f)
        {
            return 0; // outside shape
        }

        // Track the plane the player is closest to (negative values are inside the plane, closer to zero is closer to the plane)
        if (currDistance > closestPlaneDistance) // largest negative value (closest to zero)
        {
            closestPlaneDistance = currDistance;
            pushDir = normal;
        }
    }

    // offset position to be on correct side of plane
    glm::vec3 correction = pushDir * glm::abs(closestPlaneDistance);
    cam.eye += correction;

    // push player out in direction of normal
    float velocityAlongNormal = glm::dot(player.velocity, pushDir);
    if (velocityAlongNormal < 0.0f)
    {
        player.velocity -= pushDir * velocityAlongNormal;
    }

    // remove sliding
    player.velocity.y = 0;
    player.airborne = false;

    return 1; // collision detected
}