/*
	Project! - A Portal Game, with Quake styled movement or something like that

	Useful links!
	General
	https://research.ncl.ac.uk/game/mastersdegree/graphicsforgames/

	Engine Architecture
	Godot Engine Architecture - https://docs.godotengine.org/en/stable/engine_details/architecture/godot_architecture_diagram.html
	ECS - https://en.wikipedia.org/wiki/Entity_component_system#Common_patterns_in_ECS_use
	Scene Graph - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph

	Portals
	https://www.cs.rpi.edu/~cutler/classes/advancedgraphics/S21/final_projects/metzlr.pdf
	https://ajknowles11.github.io/projects/portals/
	https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/
	https://www.youtube.com/watch?v=cWpFZbjtSQg
	https://medium.com/@alikomurcu/portals-with-opengl-d74da6241dd4
	Stencil Test - https://learnopengl.com/Advanced-OpenGL/Stencil-testing
	Basic Portal - https://en.wikibooks.org/wiki/OpenGL_Programming/Mini-Portal
	Recursive Portal - https://en.wikibooks.org/wiki/OpenGL_Programming/Mini-Portal_Recursive
	Smooth Portal - https://en.wikibooks.org/wiki/OpenGL_Programming/Mini-Portal_Smooth
	Scissor Clipping - https://en.wikibooks.org/wiki/OpenGL_Programming/Mini-Portal_Optimization

	Physics
	3D Collision Detection - https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection

	Lighting
	Ray Tracing - https://raytracing.github.io/

	Post Processing
	Implementation - https://learnopengl.com/In-Practice/2D-Game/Postprocessing
	Wave Effect - https://en.wikibooks.org/wiki/OpenGL_Programming/Post-Processing

	Debug Shader
	IMGUI Explorer - https://pthom.github.io/imgui_explorer/
	Draw Bounding Box - https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
	Geom Shader to visualize Normals - https://learnopengl.com/Advanced-OpenGL/Geometry-Shader

	Optimization
	https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
*/

#include <iostream>
#include <glad/glad.h>
#include <chrono>

#include "Application.h"
#include "core/GLSL.h"
#include "core/WindowManager.h"
#include "core/Callbacks.h"
#include "renderer/Program.h"
#include "renderer/Shape.h"
#include "renderer/MatrixStack.h"
#include "renderer/Texture.h"
#include "../ext/stb_image/stb_image.h"
#include "math/Bezier.h"
#include "math/Spline.h"
#include "world/Camera.h"
#include "world/Player.h"
#include "world/GameObject.h"

// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// imgui
// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

// imgui FIXME
// https://github.com/ocornut/imgui/wiki/Getting-Started#example-if-you-are-using-glfw--openglwebgl
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"

// USEFUL RESOURCES
// https://github.com/godotengine/godot/tree/master/core/math
// https://learnopengl.com/Advanced-OpenGL/Stencil-testing

using namespace std;
using namespace glm;

int main(int argc, char *argv[])
{
	float deltaTime;

	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application(0);
	// SceneInitializer *sceneInitializer = new SceneInitializer();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(application->windowWidth, application->windowHeight);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	// sceneInitializer->initGeom(resourceDir);
	// sceneInitializer->init(resourceDir);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;				// IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(windowManager->getHandle(), true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// (Your code calls glfwPollEvents())
		// Poll for and process events.
		glfwPollEvents();

		// ...
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		static bool showDemoWindow = false;
		// ImGui::Checkbox("Show Demo Window", &showDemoWindow);
		ImGui::SliderFloat("Main Camera speed", &application->scene->cameraSpeed, -5.0f, 1000.0f);
		ImGui::SliderFloat3("Main Camera position", &application->scene->mainCamera->eye.x, -25.0f, 25.0f);
		ImGui::SliderFloat3("Player velocity", &application->scene->mainCamera->velocity.x, -1000.0f, 1000.0f);
		ImGui::SliderFloat3("Portal Camera position", &application->scene->portalCamera->eye.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Tool position", &application->scene->tool->position.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Projectile position", &application->scene->projectile->position.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Portalcube position", &application->scene->portalcube->position.x, -100.0f, 100.0f);

		ImGui::SliderFloat("g_Spin", &application->g_Spin, 0.0f, 20.0f);
		ImGui::SliderFloat3("light pos", &application->callbacks->lightTrans.x, -1000.0f, 1000.0f);

		if (showDemoWindow)
			ImGui::ShowDemoWindow(); // Show demo window! :)

		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		// Render scene.
		application->render(deltaTime);
		application->deltaTime = deltaTime;

		// imgui Rendering
		// (Your code clears your framebuffer, renders your other stuff etc.)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// (Your code calls glfwSwapBuffers() etc.)

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		// glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Quit program.
	windowManager->shutdown();
	return 0;
}