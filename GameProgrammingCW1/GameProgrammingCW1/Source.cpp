// Simplified Renderer application for GP course
// Features:
// Reduced OpenGL version from 4.5 to 3.3 to allow it to render in older laptops.
// Added Shapes library for rendering cubes, spheres and vectors.
// Added examples of matrix multiplication on Update.
// Added resize screen and keyboard callbacks.
// Added FPS camera functionality
// Update 2019/01 updated libraries and created project for VS2017 including directory dependant links to libraries.
// Update 2020/01 updated libraries for x64 and for VS2020, also adding MAC compiled Libraries.

// Suggestions or extra help please do email me at S.Padilla@hw.ac.uk

// Standard C++ libraries
#include <iostream>
#include <vector>
#include <time.h>
#include <stack>
using namespace std;

// Helper graphic libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Custom libraries
#include "graphics.h"
#include "shapes.h"
#include "Astar.h"


// MAIN FUNCTIONS
void startup();
void updateCamera();
void updateSceneElements();
void renderScene();

// CALLBACK FUNCTIONS
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);

// VARIABLES
bool        quit = false;
float       deltaTime = 0.0f;    // Keep track of time per frame.
float       lastTime = 0.0f;    // variable to keep overall time.
bool        keyStatus[1024];    // Hold key status.
bool		mouseEnabled = true; // keep track of mouse toggle.

// MAIN GRAPHICS OBJECT
Graphics    myGraphics;   

// CUSTOM DEFINITIONS

#pragma region CAMERA POSITIONS DEFINITIONS
const int MAXSCENES = 1;
int currentScene = 0;
CamTransform scenes[] = {CamTransform(glm::vec3(12.7f, 11.0f, 12.0f),-138.0f,-53.0f)};
#pragma endregion

#pragma region ASTAR DEFINITIONS
// ASTAR SHAPES
Sphere      astarAgent;
Arrow		astarGoal;
Cube        floorTiles;
Cube		walls;

// ASTAR VARIABLES
glm::vec3 agentPosition;
glm::vec3 agentTarget;
glm::vec3 agentDirection;

glm::vec3 goalArrowPosition;

std::stack<glm::vec3> aStarPath;

std::vector<glm::vec3> floorPositions;
std::vector<glm::vec3> wallPositions;

glm::mat4* wallmodels;
glm::mat4* floormodels;

//MAP

const int WIDTH = 12, HEIGHT = 12;

std::vector<std::vector<int>> map = {
					{1,1,1,1,1,1,1,1,1,1,1,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,0,1,1,0,0,0,0,0,1,0,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,0,0,0,1,0,0,0,0,0,0,1},
					{1,0,0,0,0,1,0,0,0,0,0,1},
					{1,0,0,0,0,1,1,1,0,0,0,1},
					{1,0,0,0,1,1,0,0,0,0,0,1},
					{1,0,0,0,1,0,0,0,0,1,0,1},
					{1,0,0,0,1,0,0,0,0,1,0,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,1,1,1,1,1,1,1,1,1,1,1}
};

// ASTAR FUNCTIONS
void setNewAgentTarget();
void moveAgentToTarget();
void updateAgentPosition();
#pragma endregion


// Some global variable to do the animation.
float t = 0.001f;            


// STARDARD FUNCTIONS

int main()
{
	// Launch window and graphics context
	int errorGraphics = myGraphics.Init();			
	// Close if something went wrong...
	if (errorGraphics) return 0;					

	// Setup all necessary information for startup (aka. load texture, shaders, models, etc).
	startup();										



	// MAIN LOOP run until the window is closed
	while (!quit) {

		// Update the camera transform based on interactive inputs or by following a predifined path.
		updateCamera();

		// Update position, orientations and any other relevant visual state of any dynamic elements in the scene.
		updateSceneElements();

		// Render a still frame into an off-screen frame buffer known as the backbuffer.
		renderScene();

		// Swap the back buffer with the front buffer, making the most recently rendered image visible on-screen.
		glfwSwapBuffers(myGraphics.window);  

	}

	myGraphics.endProgram();         


	return 0;
}

void startup() {

	#pragma region STANDARD SETUP CODE
	// KEEP TRACK OF THE RUNNING TIME

	// retrieve timelapse
	GLfloat currentTime = (GLfloat)glfwGetTime();
	// start delta time
	deltaTime = currentTime;
	// Save for next frame calculations.
	lastTime = currentTime;

	// CALLBACK GRAPHICS AND KEY UPDATE FUNCTIONS 
	glfwSetWindowSizeCallback(myGraphics.window, onResizeCallback);
	glfwSetKeyCallback(myGraphics.window, onKeyCallback);
	glfwSetMouseButtonCallback(myGraphics.window, onMouseButtonCallback);
	glfwSetCursorPosCallback(myGraphics.window, onMouseMoveCallback);
	glfwSetScrollCallback(myGraphics.window, onMouseWheelCallback);

	// Calculate proj_matrix for the first time.
	myGraphics.aspect = (float)myGraphics.windowWidth / (float)myGraphics.windowHeight;
	myGraphics.proj_matrix = glm::perspective(glm::radians(50.0f), myGraphics.aspect, 0.1f, 1000.0f);

	// Set first scene camera position (astar)
	myGraphics.NextScene(scenes[0]);

#pragma endregion


	// Custom Setup

	#pragma region ASTAR SETUP CODE

		// read map and create vectors for instanced objects
		int index = 0;
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
			{
				floorPositions.push_back(glm::vec3(j, 0.0f, i));
				if (map[i][j] == 1)
				{
					wallPositions.push_back(glm::vec3(j, 0.5f, i));
				}
			}
		}
	
		// FLOORS
		// init floor model matrices
		floormodels = new glm::mat4[floorPositions.size()];
		for (int i = 0; i < 144; i++)
		{
			floormodels[i] = glm::translate(floorPositions[i]) *
				glm::scale(glm::vec3(1.0f, 0.001f, 1.0f)) *
				glm::mat4(1.0f);
		}
		// Load shape floor tile
		floorTiles.LoadInstanced(&floormodels[0], WIDTH * HEIGHT);
		floorTiles.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);


		// WALLS
		// init wall model matrices
		wallmodels = new glm::mat4[wallPositions.size()];
		for (int i = 0; i < wallPositions.size(); i++)
		{
			wallmodels[i] =
				glm::translate(wallPositions[i]) *
				glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) *
				glm::mat4(1.0f);
		}

		// Load shape walls
		walls.LoadInstanced(&wallmodels[0], wallPositions.size());
		walls.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		// initialize agent
		astarAgent.Load();
		astarAgent.fillColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		agentPosition = glm::vec3(7.0f, 0.5f, 7.0f);

		// initialize goal arrow
		astarGoal.Load();
		astarGoal.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		astarGoal.lineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		goalArrowPosition = glm::vec3(6.0f, 1.0f, 4.0f);

	#pragma endregion


	// Optimised Graphics
	myGraphics.SetOptimisations();
}

void updateCamera() {

	// calculate movement for FPS camera
	GLfloat xoffset = myGraphics.mouseX - myGraphics.cameraLastX;
	GLfloat yoffset = myGraphics.cameraLastY - myGraphics.mouseY;    // Reversed mouse movement
	myGraphics.cameraLastX = (GLfloat)myGraphics.mouseX;
	myGraphics.cameraLastY = (GLfloat)myGraphics.mouseY;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	myGraphics.cameraYaw += xoffset;
	myGraphics.cameraPitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (myGraphics.cameraPitch > 89.0f) myGraphics.cameraPitch = 89.0f;
	if (myGraphics.cameraPitch < -89.0f) myGraphics.cameraPitch = -89.0f;

	// Calculating FPS camera movement (See 'Additional Reading: Yaw and Pitch to Vector Calculations' in VISION)
	glm::vec3 front;
	front.x = cos(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));
	front.y = sin(glm::radians(myGraphics.cameraPitch));
	front.z = sin(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));

	myGraphics.cameraFront = glm::normalize(front);

	// Update movement using the keys
	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W]) myGraphics.cameraPosition += cameraSpeed * myGraphics.cameraFront;
	if (keyStatus[GLFW_KEY_S]) myGraphics.cameraPosition -= cameraSpeed * myGraphics.cameraFront;
	if (keyStatus[GLFW_KEY_A]) myGraphics.cameraPosition -= glm::normalize(glm::cross(myGraphics.cameraFront, myGraphics.cameraUp)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D]) myGraphics.cameraPosition += glm::normalize(glm::cross(myGraphics.cameraFront, myGraphics.cameraUp)) * cameraSpeed;

	if (!mouseEnabled && keyStatus[GLFW_KEY_N])
	{
		currentScene = ++currentScene % MAXSCENES;
		myGraphics.NextScene(scenes[currentScene]);
		myGraphics.viewMatrix = glm::lookAt(myGraphics.cameraPosition,			// eye
			myGraphics.cameraPosition + myGraphics.cameraFront,					// centre
			myGraphics.cameraUp);
	}

	// IMPORTANT PART
	// Calculate my view matrix using the lookAt helper function
	if (mouseEnabled) {
		myGraphics.viewMatrix = glm::lookAt(myGraphics.cameraPosition,			// eye
			myGraphics.cameraPosition + myGraphics.cameraFront,					// centre
			myGraphics.cameraUp);												// up
	}
}

void updateSceneElements() {

	// poll callbacks
	glfwPollEvents();                                

	#pragma region CALCULATE FRAME TIME/PERIOD
		// retrieve timelapse
		GLfloat currentTime = (GLfloat)glfwGetTime();
		// Calculate delta time
		deltaTime = currentTime - lastTime;
		// Save for next frame calculations.
		lastTime = currentTime;
	#pragma endregion

	                          
	// Custom Updates

	#pragma region ASTAR UPDATE


			// Calculate Agent movement
			updateAgentPosition();
			if (agentTarget != glm::vec3(0))
			{
				moveAgentToTarget();
			}

			// Agent model-view-projection
			astarAgent.mv_matrix = myGraphics.viewMatrix *
				glm::translate(agentPosition) *
				glm::mat4(1.0f);
			astarAgent.proj_matrix = myGraphics.proj_matrix;

			// Goal arrow model-view-projection
			astarGoal.mv_matrix = myGraphics.viewMatrix *
				glm::translate(goalArrowPosition) *
				glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::vec3(1.5f, 1.0f, 1.5f)) *
				glm::mat4(1.0f);
			astarGoal.proj_matrix = myGraphics.proj_matrix;

			// Walls view-projection (model passed as attribute array pointers)
			walls.view_matrix = myGraphics.viewMatrix;
			walls.proj_matrix = myGraphics.proj_matrix;

			// Floors view-projection (model passed as attribute array pointer)
			floorTiles.view_matrix = myGraphics.viewMatrix;
			floorTiles.proj_matrix = myGraphics.proj_matrix;

	#pragma endregion


	// increment movement variable
	t += 0.01f; 

	// If quit by pressing x on window.
	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; 

}

void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();

	// Custom Rendering

	#pragma region ASTAR RENDER

	floorTiles.DrawInstanced(WIDTH * HEIGHT);
	walls.DrawInstanced(wallPositions.size());
	astarAgent.Draw();
	astarGoal.Draw();

#pragma endregion


}


// CUSTOM FUNCTIONS

#pragma region ASTAR FUNCTIONS

	void setNewAgentTarget()
	{
		agentTarget = aStarPath.top();
		std::cout << "moving though: " << glm::to_string(agentTarget) << std::endl;
		agentDirection = glm::normalize(agentTarget - agentPosition);
	}

	void moveAgentToTarget()
	{
		float distance = abs(glm::distance(agentTarget, agentPosition));
		if (distance > 0.1)
		{
			agentPosition += agentDirection * deltaTime * 2.0f;
		}
		else if (distance < 0.1)
		{
			agentPosition = agentTarget;
			agentTarget = glm::vec3(0);
			aStarPath.pop();
		}
	}

	void updateAgentPosition()
	{
		//if path was calculated
		if (!aStarPath.empty())
		{
			if (agentTarget == glm::vec3(0)) // path not set
			{
				setNewAgentTarget();
			}
		}
	}

	void checkAstarInput()
	{
		if (!mouseEnabled)
		{
			if (keyStatus[GLFW_KEY_UP])
			{
				goalArrowPosition.z -= 1.0f;
			}
			else if (keyStatus[GLFW_KEY_DOWN])
			{
				goalArrowPosition.z += 1.0f;
			}
			else if (keyStatus[GLFW_KEY_RIGHT])
			{
				goalArrowPosition.x += 1.0f;
			}
			else if (keyStatus[GLFW_KEY_LEFT])
			{
				goalArrowPosition.x -= 1.0f;
			}

			if (keyStatus[GLFW_KEY_ENTER])
			{
				Astar::getInstance()->setParams(WIDTH, HEIGHT, 4);
				std::cout << "agent position" << glm::to_string(agentPosition) << std::endl;
				std::cout << "goal position" << glm::to_string(goalArrowPosition) << std::endl;
				aStarPath = Astar::getInstance()->path(map, agentPosition, goalArrowPosition);
			}
		}
	}

#pragma endregion



// CALL BACKS
#pragma region CALL BACK FUNCTIONS

void onResizeCallback(GLFWwindow* window, int w, int h) {    // call everytime the window is resized
	//myGraphics.windowWidth = w;
	//myGraphics.windowHeight = h;

	glfwGetFramebufferSize(window, &myGraphics.windowWidth, &myGraphics.windowHeight);

	myGraphics.aspect = (float)w / (float)h;
	myGraphics.proj_matrix = glm::perspective(glm::radians(50.0f), myGraphics.aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { // called everytime a key is pressed
	if (action == GLFW_PRESS) keyStatus[key] = true;
	else if (action == GLFW_RELEASE) keyStatus[key] = false;

	// toggle showing mouse.
	if (keyStatus[GLFW_KEY_M]) {
		mouseEnabled = !mouseEnabled;
		myGraphics.ToggleMouse();
	}
	// If exit key pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	#pragma region CUSTOM INPUT


		switch (currentScene)
		{
			//0-ASTAR SCENE
			case 0:
				checkAstarInput();
			default:
				break;
		}
	#pragma endregion


	
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	myGraphics.mouseX = mouseX;
	myGraphics.mouseY = mouseY;

	// helper variables for FPS camera
	if (myGraphics.cameraFirstMouse) {
		myGraphics.cameraLastX = (GLfloat)myGraphics.mouseX; myGraphics.cameraLastY = (GLfloat)myGraphics.mouseY; myGraphics.cameraFirstMouse = false;
	}
}

void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int yoffsetInt = static_cast<int>(yoffset);
}
	
#pragma endregion

