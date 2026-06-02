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

#include <GLFW/glfw3.h>

// imgui
// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

// imgui FIXME
// https://github.com/ocornut/imgui/wiki/Getting-Started#example-if-you-are-using-glfw--openglwebgl
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"

// imguizmo
#include "../ext/imguizmo/ImGuizmo.h"

// implot3d
#include "../ext/implot3d/implot3d.h"
#include "../ext/implot3d/implot3d_internal.h"

// USEFUL RESOURCES
// https://github.com/godotengine/godot/tree/master/core/math
// https://learnopengl.com/Advanced-OpenGL/Stencil-testing

using namespace std;
using namespace glm;

void drawMovementGraph(Application *application);
void drawImGuizmoMenu(Application *application, WindowManager *windowManager, ImGuiIO &io, int &activeSelection);
void drawFramerateGraph(ImGuiIO &io);
void drawDebugMenu(Application *application, WindowManager *windowManager, int &activeSelection);
void drawGameHUD(ImGuiIO &io, float deltaTime);

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
	ImPlot3D::CreateContext();
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

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		// ImPlot3D
		ImGui::Begin("Movement Vectors");
		drawMovementGraph(application);
		ImGui::End();

		// Debug menu
		static int activeSelection = 1;
		drawImGuizmoMenu(application, windowManager, io, activeSelection);
		drawFramerateGraph(io);
		drawDebugMenu(application, windowManager, activeSelection);

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

		// Render HUD
		drawGameHUD(io, deltaTime);

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
	ImPlot3D::DestroyContext();
	ImGui::DestroyContext();

	// Quit program.
	windowManager->shutdown();
	return 0;
}

void drawMovementGraph(Application *application)
{
	if (ImPlot3D::BeginPlot("Movement"))
	{
		// static double xLine[2], yLine[2], zLine[2];
		ImPlot3D::SetupAxes("x", "z", "y");

		ImPlot3D::SetupAxesLimits(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, ImPlot3DCond_Once);
		glm::vec3 wishDir = application->scene->playerCamera->wishDir;

		// Graph movement
		glm::vec3 vel = application->scene->playerCamera->velocity;
		float velX[2] = {0.0f, vel.x};
		float velY[2] = {0.0f, vel.z}; // swap y and z
		float velZ[2] = {0.0f, vel.y}; // swap y and z
		ImPlot3D::PlotLine("velocity", velX, velY, velZ, 2);

		// Graph wishDir
		glm::vec3 wish = application->scene->playerCamera->wishDir;
		float wishX[2] = {0.0f, wish.x};
		float wishY[2] = {0.0f, wish.z}; // swap y and z
		float wishZ[2] = {0.0f, wish.y}; // swap y and z
		ImPlot3D::PlotLine("wishDir", wishX, wishY, wishZ, 2);

		// Graph upVector
		glm::vec3 up = glm::vec3(0, 1, 0);
		float upX[2] = {0.0f, up.x};
		float upY[2] = {0.0f, up.z}; // swap y and z
		float upZ[2] = {0.0f, up.y}; // swap y and z
		ImPlot3D::PlotLine("up", upX, upY, upZ, 2);

		ImPlot3D::EndPlot();
	}
}

void drawImGuizmoMenu(Application *application, WindowManager *windowManager, ImGuiIO &io, int &activeSelection)
{
	// Start the ImGuizmo frame
	ImGuizmo::BeginFrame();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// matrices for moving the obj
	int width, height;
	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
	float aspect = width / (float)height;

	glm::mat4 view = lookAt(application->scene->mainCamera->eye, application->scene->mainCamera->lookAtTarget, glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

	// Update objects with ImGuizmo
	if (activeSelection == 1)
	{
		glm::vec4 &lightPos = application->scene->testLight.position;
		glm::mat4 lightMat = glm::translate(glm::mat4(1.0f), glm::vec3(lightPos));

		ImGuizmo::Manipulate(glm::value_ptr(view),
							 glm::value_ptr(proj),
							 ImGuizmo::TRANSLATE,
							 ImGuizmo::WORLD,
							 glm::value_ptr(lightMat));

		if (ImGuizmo::IsUsing())
		{
			ImGui::Text("Using gizmo");

			// update cube pos
			lightPos.x = lightMat[3][0];
			lightPos.y = lightMat[3][1];
			lightPos.z = lightMat[3][2];
		}
	}
	else if (activeSelection == 2)
	{
		glm::vec3 &cubePos = application->scene->portalcube->position;
		glm::mat4 objMat = glm::translate(glm::mat4(1.0f), cubePos);

		ImGuizmo::Manipulate(glm::value_ptr(view),
							 glm::value_ptr(proj),
							 ImGuizmo::TRANSLATE,
							 ImGuizmo::WORLD,
							 glm::value_ptr(objMat));

		if (ImGuizmo::IsUsing())
		{
			ImGui::Text("Using gizmo");

			// update cube pos
			cubePos.x = objMat[3][0];
			cubePos.y = objMat[3][1];
			cubePos.z = objMat[3][2];
		}
	}
}

void drawFramerateGraph(ImGuiIO &io)
{
	// plot framerate - https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
	ImGui::Text("Performance:");

	static bool animate = true;

	// static float values[90] = {};
	static float values[300] = {}; // 5 seconds
	static int values_offset = 0;
	static double refresh_time = 0.0;

	if (!animate || refresh_time == 0.0)
		refresh_time = ImGui::GetTime();

	while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
	{
		// static float phase = 0.0f;
		// values[values_offset] = cosf(phase);
		values[values_offset] = 1000.0f / io.Framerate;
		values_offset = (values_offset + 1) % IM_COUNTOF(values);
		// phase += 0.10f * values_offset;
		refresh_time += 1.0f / 60.0;
	}

	// Plots can display overlay texts
	// (in this example, we will display an average value)
	float average = 0.0f;
	for (int n = 0; n < IM_COUNTOF(values); n++)
		average += values[n];
	average /= (float)IM_COUNTOF(values);

	char overlay[32];
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	sprintf(overlay, "avg %f ms/frame (%.1f FPS)", average, 1000.0f / average);
	ImGui::PlotLines("Frame Times", values, IM_COUNTOF(values), values_offset, overlay, 0.0f, 40.0f, ImVec2(0, 80.0f));
	ImGui::Checkbox("Animate", &animate);
}

void drawDebugMenu(Application *application, WindowManager *windowManager, int &activeSelection)
{
	// Window variables
	static bool showDemoWindow = false;

	ImGui::Separator();
	ImGui::Text("Active Gizmo Target:");
	ImGui::RadioButton("None", &activeSelection, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Light", &activeSelection, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Cube", &activeSelection, 2);

	ImGui::Separator();
	ImGui::Checkbox("Ground Collision", &application->scene->groundCollision);
	ImGui::SameLine();
	ImGui::Checkbox("Portal Cube", &application->scene->showPortalCube);
	ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &showDemoWindow);

	ImGui::Separator();
	ImGui::Text("Camera");
	ImGui::SliderFloat("Noclip speed", &application->scene->cameraSpeed, -5.0f, 1000.0f);
	ImGui::SliderFloat3("Main Camera position", &application->scene->mainCamera->eye.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("Portal Camera position", &application->scene->portalCamera->eye.x, -10.0f, 10.0f);

	ImGui::Separator();
	ImGui::Text("Player and Objects");
	ImGui::SliderFloat3("Player velocity", &application->scene->playerCamera->velocity.x, -1000.0f, 1000.0f);
	ImGui::SliderFloat3("Tool position", &application->scene->tool->position.x, -1.0f, 1.0f);
	ImGui::SliderFloat3("Projectile position", &application->scene->projectile->position.x, -1.0f, 1.0f);
	ImGui::SliderFloat3("Portalcube position", &application->scene->portalcube->position.x, -100.0f, 100.0f);

	ImGui::Separator();
	if (ImGui::TreeNode("Debug Objects"))
	{
		ImGui::Text("Debug Light");
		ImGui::SliderFloat3("color", &application->scene->testLight.color.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("intensity", &application->scene->testLight.intensity.x, 0.0f, 10.0f);
		ImGui::SliderFloat3("position", &application->scene->testLight.position.x, -100.0f, 100.0f);

		ImGui::Separator();
		ImGui::SliderFloat("g_Spin", &application->g_Spin, 0.0f, 20.0f);
		ImGui::TreePop();
	}

	if (showDemoWindow)
		ImGui::ShowDemoWindow(); // Show demo window! :)
}

void drawGameHUD(ImGuiIO &io, float deltaTime)
{
	// Set window size to be screen
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(io.DisplaySize);

	ImGuiWindowFlags hudFlags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	ImGui::Begin("##GameHUD", nullptr, hudFlags);

	// FIXME - load actual font library instead
	// io.FontGlobalScale = 2.0f;

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 screenCenter(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

	// Draw crosshair
	float crosshairSize = 8.0f;
	ImU32 crosshairColor = IM_COL32(255, 255, 255, 150);

	ImVec2 line1Start(screenCenter.x - crosshairSize, screenCenter.y);
	ImVec2 line1End(screenCenter.x + crosshairSize, screenCenter.y);
	ImVec2 line2Start(screenCenter.x, screenCenter.y - crosshairSize);
	ImVec2 line2End(screenCenter.x, screenCenter.y + crosshairSize);
	drawList->AddLine(line1Start, line1End, crosshairColor, 2.0f);
	drawList->AddLine(line2Start, line2End, crosshairColor, 2.0f);

	// Draw timer
	char timerBuffer[32];
	sprintf(timerBuffer, "TIME: %.3f", glfwGetTime());

	ImVec2 timerTextSize = ImGui::CalcTextSize(timerBuffer);
	ImU32 timerColor = IM_COL32(0, 200, 255, 200);
	ImVec2 timerPos((io.DisplaySize.x - timerTextSize.x) / 2.0f, timerTextSize.y + 10.0f); // Center x
	ImVec2 timerShadowPos(timerTextSize.x + 2.0f, timerTextSize.y + 2.0f);

	drawList->AddText(timerShadowPos, IM_COL32(0, 0, 0, 200), timerBuffer); // shadow
	drawList->AddText(timerPos, timerColor, timerBuffer);

	// Draw tutorial message
	const char *tutorialTextBuffer = R"(
	MOVEMENT CONTROLS
	[W][A][S][D]    -> Move Around
	[Space]         -> Jump, can be held
	[Shift]         -> Sprint
	[L-Click]       -> Fire
	[R-Click]       -> Reload
	[HOLD R-Click]  -> Charge
	[R]             -> Reset position
	[ESC]           -> Exit
	
	Press [~] to unlock mouse cursor)";

	float alphaFade = 255 - glfwGetTime() * 20.0f;
	ImVec2 tutorialTextSize = ImGui::CalcTextSize(tutorialTextBuffer);
	ImU32 tutorialColor = IM_COL32(255, 255, 255, alphaFade);
	ImVec2 tutorialPos((io.DisplaySize.x - tutorialTextSize.x) / 2.0f, tutorialTextSize.y + 10.0f);
	ImVec2 tutorialShadowPos(tutorialPos.x + 2.0f, tutorialPos.y + 2.0f);

	drawList->AddText(tutorialShadowPos, IM_COL32(0, 0, 0, alphaFade), tutorialTextBuffer); // shadow
	drawList->AddText(tutorialPos, tutorialColor, tutorialTextBuffer);

	ImGui::End();
}