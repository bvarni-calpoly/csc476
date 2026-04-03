// https://www.glfw.org/docs/latest/input_guide.html#cursor_pos
// https://learnopengl.com/Getting-started/Camera
// https://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#define PI 3.1415927
#include <GLFW/glfw3.h>

class Callbacks
{
private:
	/* data */
public:
	Callbacks(/* args */);
	~Callbacks();


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	void mouseCallback(GLFWwindow *window, int button, int action, int mods);
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY);
	void setCursorPosCallback(GLFWwindow* window,  double xpos, double ypos);
	void resizeCallback(GLFWwindow *window, int width, int height);

	double phi, theta; // FIXME NAMES
	//theta = -PI/2.0;
	double lastX, lastY;
};

#endif