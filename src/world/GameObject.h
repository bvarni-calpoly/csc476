#pragma once

#include <vector>
#include <glm/gtc/type_ptr.hpp>

#include "../renderer/Shape.h"

// https://docs.godotengine.org/en/stable/classes/class_scenetree.html#class-scenetree
// https://docs.godotengine.org/en/stable/classes/class_node.html#class-node
// https://github.com/godotengine/godot/blob/1aabcb9e9bc7a222a972731523831ec86b77ee20/scene/main/node.h
// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/3.2d_game/0.full_source/game_object.h
class GameObject
{

public:
	GameObject() : shape(nullptr), position(0.0f), angle(0.0f), rotation(glm::vec3(0.0f, 1.0f, 0.0f)), scale(1.0f), localMin(0.0f), localMax(0.0f) {}
	GameObject(std::shared_ptr<Shape> s, glm::vec3 pos, float ang, glm::vec3 rot, glm::vec3 scl, glm::vec3 mi, glm::vec3 ma)
		: shape(s),
		  position(pos),
		  angle(ang),
		  rotation(rot),
		  scale(scl),
		  localMin(mi),
		  localMax(ma)
	{
	}
	void addChild(std::unique_ptr<GameObject> child);
	void removeChild(int id);

	void updateBounds();

	// scene graph (godot engine)
	// std::shared_ptr<GameObject> parent; // fixme shared or raw pointer?
	int id = 0;
	bool isMarked = false;
	GameObject *parent = nullptr;					   // fixme shared or raw pointer?
	std::vector<std::unique_ptr<GameObject>> children; // fixme make hashmap? check godot docs

	std::shared_ptr<Shape> shape;
	glm::vec3 position = glm::vec3(0);
	float angle = 0;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 localMin; // Local space
	glm::vec3 localMax;
	glm::vec3 localCenter;
	// already calculate in shape
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max()); // World space
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
	
	// orientation
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 forward = glm::vec3(1, 0, 0);

	// physics
	glm::vec3 velocity = glm::vec3(0);
	int cameraCollided = 0;
	int collisionsEnabled = 1;
	int collided = 0;
};