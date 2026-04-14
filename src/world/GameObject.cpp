#include <iostream> // fixme

#include <vector>
#include <glm/gtc/type_ptr.hpp>

#include "GameObject.h"

void GameObject::updateBounds()
{
    // FIXME, account for rotations
    float normScale = 1.0f / shape->largeExtent();
    min = (localMin * normScale * scale) + position;
    max = (localMax * normScale * scale) + position;
}

void GameObject::addChild(std::unique_ptr<GameObject> child)
{
    child->parent = this; // fixme
    children.push_back(std::move(child)); // fixme
}

void GameObject::removeChild()
{
    std::cout << "GameObject::removeChild not yet implemented" << std::endl;
}
