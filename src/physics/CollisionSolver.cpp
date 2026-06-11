#include <iostream>
#include "CollisionSolver.h"
#include "../SceneInitializer.h"

int CollisionSolver::testPlayerVsAABB(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    Camera &cam = *(scene->mainCamera);
    Player &player = *(scene->playerCamera);

    // adjust for player height
    float camHeight = cam.eye.y - player.playerHeight - player.playerRadius;

    // check each axis for collision
    bool xCollision = (cam.eye.x > obj.min.x) && (cam.eye.x < obj.max.x);
    bool yCollision = (cam.eye.y > obj.min.y) && (camHeight < obj.max.y);
    bool zCollision = (cam.eye.z > obj.min.z) && (cam.eye.z < obj.max.z);

    if (xCollision && yCollision && zCollision)
        return 1; // collision detected

    return 0; // no collision
}

int CollisionSolver::testAABBvsAABB(const GameObject &obj1, const GameObject &obj2)
{
    // check each axis for collision
    bool xCollision = (obj1.max.x >= obj2.min.x) && (obj1.min.x <= obj2.max.x);
    bool yCollision = (obj1.max.y >= obj2.min.y) && (obj1.min.y <= obj2.max.y);
    bool zCollision = (obj1.max.z >= obj2.min.z) && (obj1.min.z <= obj2.max.z);

    if (xCollision && yCollision && zCollision)
        return 1; // collision detected

    return 0; // no collision
}

float CollisionSolver::solveRaycastVsAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const GameObject &obj)
{

    // P(t) = RayOrigin + t * RayDir
    // xmin = RayOrigin.x + t * RayDir.x

    // distance from ray origin to obj, divide by rayDir to project vector against rayDir
    glm::vec3 t1 = (obj.min - rayOrigin) / rayDir;
    glm::vec3 t2 = (obj.max - rayOrigin) / rayDir;

    // Sort entry and exit of ray
    glm::vec3 tNear = glm::min(t1, t2);
    glm::vec3 tFar = glm::max(t1, t2);

    float tMin = glm::max(glm::max(tNear.x, tNear.y), tNear.z);
    float tMax = glm::min(glm::min(tFar.x, tFar.y), tFar.z);

    if (tMin <= tMax && tMax >= 0.0f)
        return tMin; // distance from rayOrigin

    return -1.0f;
    // return tMin <= tMax && tMax >= 0.0f; // distance from rayOrigin
}

int CollisionSolver::testPlayerVsPlane(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
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
    // float threshold = 10.0f; // threshold to check inbetween distances
    // if (obj.horizontal)
    //     threshold = 100.0f;
    float threshold = 10.0f;       // threshold to check inbetween distances
    if (obj.portal->portalID <= 2) // FIXME HARDCODED - horizontal (falling) portal
        threshold = 1000.0f;
    if (distance <= 0.0f && distance >= -threshold)
    {
        // std::cout << "infinite plane collision detected" << std::endl;

        if (testPlayerVsAABB(scene, obj))
            return 1; // collision detected
    }

    return 0; // no collision
}

int CollisionSolver::testObjVsPlane(const GameObject &obj, const GameObject &objPortal)
{
    // vertex normal
    glm::vec3 normal = glm::normalize(objPortal.normals[0]);

    // --- distance = (N * P) + d ---
    // Calculate position of plane relative to normal
    float d = -glm::dot(normal, objPortal.position);

    // adjust for player height
    glm::vec3 height = glm::vec3(0, obj.max.y - obj.min.y, 0);

    // Distance from camera to plane
    float distance = glm::dot(normal, height) + d;

    // check if on other side of plane
    // float threshold = 10.0f; // threshold to check inbetween distances
    // if (obj.horizontal)
    //     threshold = 100.0f;
    float threshold = 10.0f; // threshold to check inbetween distances
    if (distance <= 0.0f && distance >= -threshold)
    {
        // std::cout << "infinite plane collision detected" << std::endl;

        // if (testPlayerVsAABB(scene, obj))
        return 1; // collision detected
    }

    return 0; // no collision
}

CollisionPlaneResult CollisionSolver::solvePointVsConvex(const glm::vec3 point, const GameObject &obj) // FIXME optimize this, check godot docs
{
    CollisionPlaneResult result;
    result.collided = 0;
    result.normal = glm::vec3(0.0f);
    result.planeDistance = 0.0f;

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
        float currDistance = glm::dot(normal, point) + d;

        // Check if outside the plane
        if (currDistance > 0.0f)
        {
            return result; // outside shape
        }

        // Track the plane the player is closest to (negative values are inside the plane, closer to zero is closer to the plane)
        if (currDistance > closestPlaneDistance) // largest negative value (closest to zero)
        {
            closestPlaneDistance = currDistance;
            pushDir = normal;
        }
    }

    result.collided = 1;
    result.normal = pushDir;
    result.planeDistance = closestPlaneDistance;

    return result; // collision detected
}

/**
 * https://en.wikipedia.org/wiki/Convex_hull
 * Tests every plane on a convex shape to see if point is inside the shape
 *
 * If the player is inside the convex shape, distance is negative
 * If the player is outside the convex shape, distance is positive
 */
CollisionPlaneResult CollisionSolver::solvePlayerVsConvex(std::shared_ptr<SceneInitializer> &scene, const GameObject &obj) // FIXME optimize this, check godot docs
{
    CollisionPlaneResult result;
    result.collided = 0;
    result.normal = glm::vec3(0.0f);
    result.planeDistance = 0.0f;

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

        float effectiveDistance = currDistance - player.playerRadius;

        // Check if outside the plane
        if (effectiveDistance > 0.0f)
        {
            return result; // outside shape
        }

        // Track the plane the player is closest to (negative values are inside the plane, closer to zero is closer to the plane)
        if (effectiveDistance > closestPlaneDistance) // largest negative value (closest to zero)
        {
            closestPlaneDistance = effectiveDistance;
            pushDir = normal;
        }
    }

    result.collided = 1;
    result.normal = pushDir;
    result.planeDistance = closestPlaneDistance;

    return result; // collision detected
}