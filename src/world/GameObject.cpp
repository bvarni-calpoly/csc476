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

    localCenter = (localMax + localMin) / 2.0f;
}

void GameObject::updateBoundsMapGeom()
{
    min = localMin;
    max = localMax;

    localCenter = (localMax + localMin) / 2.0f;
}

void GameObject::addChild(std::unique_ptr<GameObject> child)
{
    child->parent = this; // fixme
    children.push_back(std::move(child)); // fixme
}

void GameObject::removeChild(int id) // FIXME
{
    for(int i = 0; i < children.size(); i++)
    {
        if(children[i]->id == id)
        {
            isMarked = true;
        }
    }
}
