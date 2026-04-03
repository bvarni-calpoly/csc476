/*
    Project!
*/

#include <iostream>
#include <glad/glad.h>
#include <chrono>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "math/Bezier.h"
#include "math/Spline.h"
#include "physics/CameraMovement.h"
#include "GameObject.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

float deltaTime;

int windowWidth = 1280;
int windowHeight = 960;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong has diffuse
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg; // Our shader program for textures

	//our geometry
	shared_ptr<Shape> cube;
	vector<Shape> cubeObject;
	shared_ptr<Shape> skybox;
	vector<GameObject> skyboxObject;
	//vector<shared_ptr<Shape>> testShape;

	vec3 skyboxMin, skyboxMax;
	vec3 gMin;
	
	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;

	int windowWidth = 1280;
	int windowHeight = 960;

	//animation data
	float lightTrans = 0;
	int g_Mat = 0;
	float g_Spin = 3.14/180 * 2; // 2 deg
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;

	//camera
	int cameraControl = 0;
	double g_phi, g_theta;
	double lastX, lastY;

	//player
	Spline splinepath[4];
	bool goCamera = false;
	double gravity = 0.2;
	vec3 velocity = vec3(0.0f);

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// might not work on WSL?
		// glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);   

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		g_theta = -PI/2.0;

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shaders/mat_vert.glsl", resourceDirectory + "/shaders/mat_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex"); //silence error

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/shaders/tex_vert.glsl", resourceDirectory + "/shaders/tex_frag.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		//read in a load the texture
		texture0 = make_shared<Texture>();
  		texture0->setFilename(resourceDirectory + "/image.jpg");
  		texture0->init();
  		texture0->setUnit(0);
  		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  		texture1 = make_shared<Texture>();
  		texture1->setFilename(resourceDirectory + "/image.jpg");
  		texture1->init();
  		texture1->setUnit(1);
  		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  		// init splines up and down
       splinepath[0] = Spline(glm::vec3(0,2,0), glm::vec3(-5,2,0), glm::vec3(5,2,0), glm::vec3(0,4,-2), 2);
       splinepath[1] = Spline(glm::vec3(0,4,-2), glm::vec3(8,6,0), glm::vec3(0,4,2), 2);
       splinepath[2] = Spline(glm::vec3(0,4,2), glm::vec3(-8,6,0), glm::vec3(0,4,-2), 2);
       splinepath[3] = Spline(glm::vec3(0,4,-2), glm::vec3(1,2,0), glm::vec3(-1,2,0), glm::vec3(0,2,0), 2);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
		
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/objects/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cube = make_shared<Shape>();
			cube->createShape(TOshapes[0]);
			cube->measure();
			cube->init();
		}

		// Initialize skybox mesh
		vector<tinyobj::shape_t> TOshapesSkybox;
 		vector<tinyobj::material_t> objMaterialsSkybox;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesSkybox, objMaterialsSkybox, errStr, (resourceDirectory + "/objects/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			skybox = make_shared<Shape>();
			skybox->createShape(TOshapesSkybox[0]);
			skybox->measure();
			skybox->init();
			skybox->center = (skybox->max + skybox->min)/2.0f; // FIXME
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	//directly pass quad for the ground to the GPU
	void initGround()
	{
		float g_groundSize = 20;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    }

    //code to draw the ground plane
    void drawGround(shared_ptr<Program> curS)
	{
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
     	texture0->bind(curS->getUniform("Texture0"));
		//draw the ground plane 
  		SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->unbind();
    }

    void drawSkybox(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, shared_ptr<Shape> shape) {
     	curS->bind();
		glUniform1i(curS->getUniform("flip"), 0);
     	texture1->bind(curS->getUniform("Texture0"));

        /*
		skyboxObject[0] = GameObject( // fixme
			skybox,
			vec3(0),
			vec3(0),
			vec3(0),
			skybox->min,
			skybox->max
		);
        */

		//skyboxObject[0].updateBounds();

		//draw the skybox
		Model->pushMatrix();
			Model->loadIdentity();

			float halfHeight = (shape->max.y - shape->min.y) / 2.0f;

			Model->scale(vec3(50.0f));
			Model->rotate(g_Spin * glfwGetTime(), vec3(0, -1, 0));
			Model->translate(vec3(0, -0.005f, 0)); // translate down 0.01 (so ground shows up)

			// normalize
			Model->scale(1.0/shape->largeExtent());
			Model->translate(vec3(0, halfHeight, 0)); // move to ground (half of height)
			Model->translate(-shape->center); // move to origin

			setModel(curS, Model);
			shape->draw(curS);
		Model->popMatrix();
  		curS->unbind();
    }

    //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i)
	{
    	switch (i) {
    		case 0: //purple
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
			case 1:
				// https://learnopengl.com/Lighting/Materials
				glUniform3f(curS->getUniform("MatAmb"), sin(glfwGetTime() * 2.0f)/4 + 0.52, sin(glfwGetTime() * 0.7f)/4 + 0.25, sin(glfwGetTime() * 1.3f)/4 + 0.25);
				glUniform3f(curS->getUniform("MatDif"), sin(glfwGetTime() / 4.0f)/4 + 0.52, sin(glfwGetTime() / 4.0f)/4 + 0.25, sin(glfwGetTime() / 4.0f)/4 + 0.25);
				glUniform3f(curS->getUniform("MatSpec"), sin(glfwGetTime() / 4.0f)/4 + 0.52, sin(glfwGetTime() / 4.0f)/4 + 0.25, sin(glfwGetTime() / 4.0f)/4 + 0.25);
				glUniform1f(curS->getUniform("MatShine"), 150.0);
			break;
  		}
	}

	/* helper function to set model transforms */
  	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

   	/* draws static hier model */
   	void drawHierModel(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, vector<shared_ptr<Shape>> Shape, vec3 min, vec3 max, int material = 0, vec3 trans = vec3(0.0), float rotateDeg = 0, vec3 rotate = vec3(0.0), vec3 scale = vec3(1.0))
	{
   		// set material
		//SetMaterial(prog, material);
		
		// draw hierarchical mesh
		Model->pushMatrix();
			Model->loadIdentity();

			float halfHeight = (max.y - min.y) / 2.0f;
			vec3 center = (max + min) / 2.0f;

			// offset
			Model->translate(trans);
			Model->rotate(rotateDeg, rotate);
			Model->scale(scale);

			// normalize
			Model->scale(1.0/(max - min).y);
			Model->translate(vec3(0, halfHeight, 0)); // move to ground
			Model->translate(-center);

			// render each part
			for(auto part : Shape) {
				setModel(curS, Model);
				part->draw(curS);
			}
			
		Model->popMatrix();
   	}

	int ObjectCollision_AABB(GameObject &object1, GameObject &object2)
	{
        cout << "Collision not implemented" << endl;

		return 0;
	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		//update the camera position
		updateUsingCameraPath(frametime);

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);
		
		// use the texture shader
		texProg->bind();
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			SetView(texProg);
			glUniform3f(texProg->getUniform("lightPos"), 2.0+lightTrans, 5.0, 2.9);
			glUniform1f(texProg->getUniform("MatShine"), 27.9);
			glUniform1i(texProg->getUniform("flip"), 1);
			texture1->bind(texProg->getUniform("Texture0"));
			Model->pushMatrix();

			glUniform1i(texProg->getUniform("flip"), 0);
			//drawGround(texProg);
			//drawSkybox(texProg, Model, skybox);

			Model->popMatrix();
		texProg->unbind();

		//use the material shader
		prog->bind();
			//set up all the matrices
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			SetView(prog);
			glUniform3f(prog->getUniform("lightPos"), 2.0+lightTrans, 2.0, 2.9);
			Model->pushMatrix();
				Model->loadIdentity();

				SetMaterial(prog, 0);

				Model->rotate(g_Spin * glfwGetTime(), vec3(0, -1, 0));
				// normalize
				Model->scale(1.0/skybox->largeExtent());
				Model->translate(vec3(0, 0, 0)); // move to ground (half of height)

				setModel(prog, Model);
				skybox->draw(prog);
			Model->popMatrix();
		prog->unbind();

		// --- CAMERA AND COLLISION LOGIC --- FIXME
		// animation updates
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

		// save previous camera position for collision
		g_eye_prev = g_eye;

		// camera movements
		if(cameraControl == 0)
			cameraMovement(windowManager->getHandle(), 3.0); // smooth camera movements
		else
			playerMovement(windowManager->getHandle(), 3.0); // control the player
		
		g_lookAt = g_eye + g_forward; // FIXME, put this before?

		// -- COLLISION CHECKING ---
		int collided = 0;

		// check collisions for crate
		//for(collided = 0; auto& object : skyboxObject)
		//	if(collided == 0)
		//		collided = CameraCollision_AABB(object);
		//	else
		//		break;
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(windowWidth, windowHeight);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
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
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}