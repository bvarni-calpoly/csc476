// https://www.glfw.org/docs/latest/input_guide.html#cursor_pos
// https://learnopengl.com/Getting-started/Camera
// https://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#define PI 3.1415927
#include <GLFW/glfw3.h>
#include "../world/Camera.h"

class Callbacks
{
private:
	/* data */
public:
	Callbacks(Camera *cam);
	~Callbacks();

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	void mouseCallback(GLFWwindow *window, int button, int action, int mods);
	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY);
	void setCursorPosCallback(GLFWwindow *window, double xpos, double ypos);
	void resizeCallback(GLFWwindow *window, int width, int height);

	Camera *camera;
	double phi, theta; // FIXME NAMES
	// theta = -PI/2.0;
	double lastX, lastY;
	glm::vec3 lightTrans = glm::vec3(0.0f, 10.0f, 10.0f);
	bool cinematicCamera = false;
	bool freeCamera = true;
	int mat = 0;
	bool mouseEnabled = true;
};

#endif