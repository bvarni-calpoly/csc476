#include <vector>
#include <glm/gtc/type_ptr.hpp>

#include "GameObject.h"

void GameObject::updateBounds()
{
    float normScale = 1.0f/shape->largeExtent(); 
    min = (localMin * normScale * scale) + position;
    max = (localMax * normScale * scale) + position;
}