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
#include "Body.h"
#include "Player.h"

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

glm::vec3 playerSceneOffset(100.0f, 0.0f, 100.0f);
glm::vec3 boidsSceneOffset(200.0f, 0.0f, 200.0f);

bool nextScenePressed = false;
const int MAXSCENES = 3;
int currentScene = 0;
CamTransform scenes[] = {
	CamTransform(glm::vec3(12.7f, 11.0f, 12.0f),-138.0f,-53.0f),
	CamTransform(glm::vec3(105.7f, 14.0f, 95.0f),-270.0f,-51.0f),
	CamTransform(glm::vec3(206.0f, 10.5f, 198.0f),-270.0f,-55.0f)
};
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

#pragma region PLAYER DEFINITIONS

Cube player;
Body playerBody(glm::vec3(110.0f, 0.5f, 104.0f), glm::vec3(0.0f), glm::vec3(1.0f));

std::vector<std::vector<int>> playerGrid = {
					{1,1,1,1,1,1,1,1,1,1,1,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,0,1,1,0,0,0,0,0,1,0,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,0,0,0,0,0,0,0,0,0,0,1},
					{1,0,0,0,0,0,0,0,0,0,0,1},
					{1,0,0,0,0,0,0,0,0,0,0,1},
					{1,0,0,0,1,0,0,0,0,0,0,1},
					{1,0,0,0,1,0,0,0,0,0,0,1},
					{1,0,0,0,1,0,0,0,0,1,0,1},
					{1,0,0,0,0,0,0,0,0,1,0,1},
					{1,1,1,1,1,1,1,1,1,1,1,1}
};
//Grid Floor
Cube playerGridFloor;
glm::vec3 playerGridFloorScale(12.0f, 0.001f, 12.0f);

// Grid bodies (instance rendering)
Cube playerGridObstacles;
std::vector<Body> playerGridObstacleBodies;

glm::mat4* playerGridObstaclesModels;


// Player Functions
void checkPlayerInputs();
bool CheckCollision(Body& obj1, Body& obj2);
#pragma endregion

#pragma region BOIDS DEFINITIONS

const int numBoids = 100;

// Boid controller 
Arrow boidsController;
glm::vec3 boidsControllerPosition(210.0f, 0.5f, 204.0f);


// Boids (instance rendering)
Sphere boids;
std::vector<Body> boidsBodies;
glm::mat4 boidsModels[numBoids];

// Boids Grid
std::vector<std::vector<int>> boidsGrid = {
					{0,0,0,0,0,0,0,0,0,0,0,0},
					{0,0,0,0,0,0,0,0,0,1,0,0},
					{0,0,1,1,0,0,0,0,0,1,0,0},
					{0,0,0,0,0,0,0,0,0,1,0,0},
					{0,0,0,0,0,0,0,0,0,0,0,0},
					{0,0,0,0,0,0,0,0,0,0,0,0},
					{0,0,0,0,0,0,0,0,0,0,0,0},
					{0,0,0,0,1,0,0,0,0,0,0,0},
					{0,0,0,0,1,0,0,0,0,0,0,0},
					{0,0,0,0,1,0,0,0,0,1,0,0},
					{0,0,0,0,0,0,0,0,0,1,0,0},
					{0,0,0,0,0,0,0,0,0,0,0,0}
};

//Grid floor
Cube boidsgridFloor;
glm::vec3 boidsGridFloorScale(12.0f, 0.001f, 12.0f);

// Grid bodies (instance rendering)
Cube boidsGridObstacles;
std::vector<Body> boidsGridObstacleBodies;

glm::mat4* boidsGridObstaclesModels;

// FUNCTIONS
void BoidRule1();
void BoidRule2();
void BoidRule3();

#pragma endregion

#pragma region PARTICLES DEFINITIONS

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

	#pragma region PLAYER SETUP CODE

		// GRID INIT

		// Floor
		playerGridFloor.Load();
		playerGridFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
		playerGridFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again

		// Walls
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
			{
				if (playerGrid[i][j] == 1)
				{
					Body body(glm::vec3(j + playerSceneOffset.x, 0.5f, i + playerSceneOffset.z), glm::vec3(0.0f), glm::vec3(1.0f));
					body.isStatic = true;
					playerGridObstacleBodies.push_back(body);
				}
			}
		}

		playerGridObstaclesModels = new glm::mat4[playerGridObstacleBodies.size()];
		for (int i = 0; i < playerGridObstacleBodies.size(); i++)
		{
			playerGridObstaclesModels[i] = glm::translate(playerGridObstacleBodies[i].position) *
				glm::scale(playerGridObstacleBodies[i].scale) *
				glm::mat4(1.0f);
		}
		playerGridObstacles.LoadInstanced(&playerGridObstaclesModels[0], playerGridObstacleBodies.size());
		

		// Player
		player.Load();

		player.fillColor = glm::vec4(0.1f, 1.0f, 0.3f, 1.0f);

	#pragma endregion

	#pragma region BOIDS SETUP CODE
		// GRID INIT

		// Floor
		boidsgridFloor.Load();
		boidsgridFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
		boidsgridFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again

		// Walls
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
			{
				if (boidsGrid[i][j] == 1)
				{
					Body body(glm::vec3(j + boidsSceneOffset.x, 0.5f, i + boidsSceneOffset.z), glm::vec3(0.0f), glm::vec3(1.0f));
					body.isStatic = true;
					boidsGridObstacleBodies.push_back(body);
				}
			}
		}

		boidsGridObstaclesModels = new glm::mat4[playerGridObstacleBodies.size()];
		for (int i = 0; i < boidsGridObstacleBodies.size(); i++)
		{
			boidsGridObstaclesModels[i] = glm::translate(boidsGridObstacleBodies[i].position) *
				glm::scale(playerGridObstacleBodies[i].scale) *
				glm::mat4(1.0f);
		}
		boidsGridObstacles.LoadInstanced(&boidsGridObstaclesModels[0], numBoids);

		// Controller
		boidsController.Load();
		boidsController.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		boidsController.lineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		// Boids

		for (int i = 0; i < numBoids; i++)
		{
			Body body(glm::vec3(boidsSceneOffset.x + rand() % 10 + 1, 0.5f, boidsSceneOffset.z + rand() % 10 + 1), glm::vec3(0.0f), glm::vec3(1.0f));
			body.debugSpeed = 1;
			body.isBoid = true;
			body.topSpeed = 0.2f;
			boidsBodies.push_back(body);

			//init model matrix
			boidsModels[i] = glm::translate(body.position) *
				glm::scale(body.scale) *
				glm::mat4(1.0f);
		}

		boids.LoadInstanced(&boidsModels[0], numBoids);
		boids.fillColor = glm::vec4(0.3f, 1.0f, 0.2f, 1.0f);
	#pragma endregion

	#pragma region PARTICLES SETUP CODE

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

	if (!mouseEnabled && keyStatus[GLFW_KEY_N] && !nextScenePressed)
	{	
		nextScenePressed = true;
		currentScene = ++currentScene % MAXSCENES;

		myGraphics.NextScene(scenes[currentScene]);

		glm::vec3 front;
		front.x = cos(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));
		front.y = sin(glm::radians(myGraphics.cameraPitch));
		front.z = sin(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));

		myGraphics.cameraFront = glm::normalize(front);

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

	#pragma region PLAYER UPDATE

				// GRID
				// Floor
				playerGridFloor.mv_matrix = myGraphics.viewMatrix *
					glm::translate(playerSceneOffset + glm::vec3(WIDTH/2,0.0f,HEIGHT/2)) *
					glm::scale(playerGridFloorScale) *
					glm::mat4(1.0f);
				playerGridFloor.proj_matrix = myGraphics.proj_matrix;

				// Walls
			
				playerGridObstacles.view_matrix = myGraphics.viewMatrix;
				playerGridObstacles.proj_matrix = myGraphics.proj_matrix;

				// PLAYER
				//Check collision
				for (int i = 0; i < playerGridObstacleBodies.size(); i++) {
					CheckCollision(playerBody, playerGridObstacleBodies[i]);
				}

				// Update physics
				playerBody.Update(deltaTime);

				// Update model-view-projection
				player.mv_matrix = myGraphics.viewMatrix *
					glm::translate(playerBody.position) *
					glm::mat4(1.0f);
				player.proj_matrix = myGraphics.proj_matrix;


	#pragma endregion

	#pragma region BOIDS UPDATE

				// floor
				boidsgridFloor.mv_matrix = myGraphics.viewMatrix *
					glm::translate(boidsSceneOffset + glm::vec3(WIDTH / 2, 0.0f, HEIGHT / 2)) *
					glm::scale(boidsGridFloorScale) *
					glm::mat4(1.0f);
				boidsgridFloor.proj_matrix = myGraphics.proj_matrix;

				// obstacles
				boidsGridObstacles.view_matrix = myGraphics.viewMatrix;
				boidsGridObstacles.proj_matrix = myGraphics.proj_matrix;

				// controller model-view-projection
				boidsController.mv_matrix = myGraphics.viewMatrix *
					glm::translate(boidsControllerPosition) *
					glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
					glm::scale(glm::vec3(1.5f, 1.0f, 1.5f)) *
					glm::mat4(1.0f);
				boidsController.proj_matrix = myGraphics.proj_matrix;

				// boids updates
				BoidRule1();
				BoidRule2();
				BoidRule3();

				for (int i = 0; i < boidsBodies.size(); i++)
				{
					boidsBodies[i].velocity += boidsBodies[i].boidVelocity;
					boidsBodies[i].Update(deltaTime);
					boidsBodies[i].boidVelocity = glm::vec3(0, 0, 0);
				}

				for (int i = 0; i < boidsBodies.size(); i++) {
					for (int j = 0; j < boidsGridObstacleBodies.size(); j++) {
						CheckCollision(boidsBodies[i], boidsGridObstacleBodies[j]);
					}
				}

				// boids view-projection
				boids.view_matrix = myGraphics.viewMatrix;
				boids.proj_matrix = myGraphics.proj_matrix;


				// boids model
				for (int i = 0; i < numBoids; i++)
				{
					boidsModels[i] = glm::translate(boidsBodies[i].position) *
						glm::mat4(1.0f);
				}
				
				boids.UpdateModelBuffer(&boidsModels[0], numBoids);
			
	#pragma endregion

#pragma region PARTICLES UPDATE

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

	#pragma region PLAYER RENDER

		// GRID
		playerGridFloor.Draw();
		playerGridObstacles.DrawInstanced(playerGridObstacleBodies.size());

		// PLAYER
		player.Draw();

	#pragma endregion

	#pragma region BOIDS RENDER

		// GRID
		boidsgridFloor.Draw();
		boidsGridObstacles.DrawInstanced(boidsGridObstacleBodies.size());

		// CONTROLLER
		boidsController.Draw();
		// BOIDS
		boids.DrawInstanced(numBoids);

	#pragma endregion

#pragma region PARTICLE RENDER

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

#pragma endregion

#pragma region COLLISION FUNCTIONS

	bool CheckCollision(Body& obj1, Body& obj2)
	{
		if (obj1.left < obj2.right && obj1.right > obj2.left && obj1.up > obj2.down&& obj1.down < obj2.up)
		{
			obj1.isColliding = true;
			if (obj1.isBoid && obj2.isBoid)
			{
				obj2.isColliding = true;
				obj1.addedForce = (obj1.velocity * glm::vec3(1.5f, 1.5f, 1.5f));// *glm::vec3(-1, -1, -1);
				obj2.addedForce = (obj1.velocity * glm::vec3(-1, -1, -1));
				if (obj1.position.y < 1)
				{
					obj1.addedForce.y = 10;
				}
				return true;
			}
			else if (obj1.isBoid && !obj2.isBoid)
			{
				obj1.addedForce = (obj1.velocity * glm::vec3(1.5f, 1.5f, 1.5f));// *glm::vec3(-1, -1, -1);
				return true;
			}

			obj1.isColliding = true;
			if (obj1.direction == Direction::Left && obj2.direction == Direction::Idle)
			{
				obj1.addedForce = glm::vec3(-5, 0, 0);
				printf("COLLIDING LEFT\n");

			}
			else if (obj1.direction == Direction::Right && obj2.direction == Direction::Idle)
			{
				obj1.addedForce = glm::vec3(5, 0, 0);
				printf("COLLIDING RIGHT\n");

			}
			else if (obj1.direction == Direction::Up && obj2.direction ==Direction::Idle)
			{
				obj1.addedForce = glm::vec3(0, 0, -5);
				printf("COLLIDING UP\n");
			}
			else if (obj1.direction == Direction::Down && obj2.direction == Direction::Idle)
			{
				obj1.addedForce = glm::vec3(0, 0, 5);
				printf("COLLIDING DOWN \n");
			}
			return true;
		}
		obj1.isColliding = false;
		obj2.isColliding = false;
		return false;
	}

#pragma endregion

#pragma region BOIDS FUNCTIONS

	glm::vec3 GetBoidsCenter()
	{
		glm::vec3 temp = glm::vec3(0, 0, 0);
		for (int i = 0; i < boidsBodies.size(); i++)
		{
			temp += boidsBodies[i].position;
		}

		glm::vec3 temp2 = glm::vec3(temp.x / boidsBodies.size(), temp.y / boidsBodies.size(), temp.z / boidsBodies.size());
		return temp2;
	}

	void BoidRule1()
	{
		for (int i = 0; i < boidsBodies.size(); i++)
		{
			boidsBodies[i].boidVelocity += (GetBoidsCenter() - boidsBodies[i].position) / glm::vec3(100, 100, 100);
			boidsBodies[i].boidVelocity += (boidsControllerPosition - boidsBodies[i].position) / glm::vec3(8, 8, 8);
		}

	}

	void BoidRule2()
	{
		for (int i = 0; i < boidsBodies.size(); i++)
		{
			if (boidsBodies[i].position.y < 1)
			{
				boidsBodies[i].boidVelocity += 1;
			}

			for (int j = 0; j < boidsBodies.size(); j++)
			{
				if (i != j)
				{
					if (glm::distance(boidsBodies[i].position, boidsBodies[j].position) < 1)
					{
						boidsBodies[i].boidVelocity += boidsBodies[i].position - boidsBodies[j].position;
					}
				}
			}

			for (int j = 0; j < boidsGridObstacleBodies.size(); j++)
			{
				if (glm::distance(boidsBodies[i].position, boidsGridObstacleBodies[j].position) < 1.2f)
				{
					boidsBodies[i].boidVelocity += boidsBodies[i].position - boidsGridObstacleBodies[j].position;
				}
			}

		}
	}

	void BoidRule3()
	{
		glm::vec3 tempVel = glm::vec3(0, 0, 0);
		for (int i = 0; i < boidsBodies.size(); i++)
		{
			for (int j = 0; j < boidsBodies.size(); j++)
			{
				tempVel += boidsBodies[j].velocity;
			}

			tempVel = glm::vec3(tempVel.x / boidsBodies.size(), tempVel.y / boidsBodies.size(), tempVel.z / boidsBodies.size());
			boidsBodies[i].boidVelocity += tempVel / glm::vec3(8, 8, 8);
		}
	}

#pragma endregion

#pragma region INPUT FUNCTIONS

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

	void checkPlayerInputs()
	{
		if (keyStatus[GLFW_KEY_UP])
		{
			playerBody.velocity.x = 0.0f;
			if (playerBody.velocity.z < 0) playerBody.velocity.z = 0.0f;
			playerBody.acceleration.z = 0.1f;
		}
		else if (keyStatus[GLFW_KEY_DOWN]) {
			playerBody.velocity.x = 0.0f;
			if (playerBody.velocity.z > 0) playerBody.velocity.z = 0.0f;
			playerBody.acceleration.z = -0.1f;
		}
		else if (keyStatus[GLFW_KEY_RIGHT]) {
			playerBody.velocity.z = 0.0f;
			if (playerBody.velocity.x > 0) playerBody.velocity.x = 0.0f;
			playerBody.acceleration.x = -0.1f;
		}
		else if (keyStatus[GLFW_KEY_LEFT]) {
			playerBody.velocity.z = 0.0f;
			if (playerBody.velocity.x < 0) playerBody.velocity.x = 0.0f;
			playerBody.acceleration.x = 0.1f;
		}
		else {
			playerBody.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			playerBody.acceleration.x = 0.0f;
			playerBody.acceleration.z = 0.0f;
		}
	}

	void checkBoidInputs()
	{
		if (keyStatus[GLFW_KEY_UP])
		{
			boidsControllerPosition.z += 0.2f;
		}
		else if (keyStatus[GLFW_KEY_DOWN]) {
			boidsControllerPosition.z -= 0.2f;
		}
		else if (keyStatus[GLFW_KEY_RIGHT]) {
			boidsControllerPosition.x -= 0.2f;
		}
		else if (keyStatus[GLFW_KEY_LEFT]) {
			boidsControllerPosition.x += 0.2f;
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

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	#pragma region STANDARD INPUTS
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
#pragma endregion

	#pragma region CUSTOM INPUT
		
		if (action == GLFW_RELEASE && key == GLFW_KEY_N && nextScenePressed)
		{
			nextScenePressed = false;
		}

		if (keyStatus[GLFW_KEY_C])
		{
			std::cout << "CAM position: " << glm::to_string(myGraphics.cameraPosition) << std::endl;
			std::cout << "CAM yaw: " << myGraphics.cameraYaw << std::endl;
			std::cout << "CAM pitch: " << myGraphics.cameraPitch << std::endl;
		}
		
		switch (currentScene)
		{
			// 0- ASTAR SCENE
			case 0:
				checkAstarInput();
				break;
			// 1- PLAYER SCENE
			case 1:
				checkPlayerInputs();
				break;
			// 2- BOIDS SCENE
			case 2:
				checkBoidInputs();
				break;
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

