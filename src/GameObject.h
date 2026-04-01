#pragma once

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"

// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/3.2d_game/0.full_source/game_object.h
class GameObject
{

public:
	GameObject(): shape(nullptr), position(0.0f), rotation(0.0f), scale(0.0f), min(0.0f), max(0.0f) {}
	GameObject(std::shared_ptr<Shape> s, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl, glm::vec3 mi, glm::vec3 ma):
		shape(s),
		position(pos),
		rotation(rot),
		scale(scl),
		localMin(mi),
		localMax(ma)
	{}

	void updateBounds();

    std::shared_ptr<Shape> shape;
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(0);
	glm::vec3 localMin = glm::vec3(0); // Local space
	glm::vec3 localMax = glm::vec3(0);
	glm::vec3 min = glm::vec3(0); // World space
	glm::vec3 max = glm::vec3(0);
	
};

#endif