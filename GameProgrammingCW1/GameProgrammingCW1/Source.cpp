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
using namespace std;

// Helper graphic libraries
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "graphics.h"
#include "shapes.h"
#include "Astar.h"
#include "ParticleSystem.h"
#include <stack>

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
Graphics    myGraphics;        // Runing all the graphics in this object

ParticleSystem ps(3000, glm::vec3(0.0f, 2.0f, 0.0f));

// SCENE OBJECTS
Sphere      mySphere;
Arrow		myArrow;
Cube        myFloor;
Cube		myWall;

Quad test;
// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation

const int WIDTH = 12, HEIGHT = 12;

glm::vec3 floorPositions [WIDTH*HEIGHT];
std::vector<glm::vec3> wallPositions;
glm::vec3 goalArrowPosition;
glm::vec3 agentPosition;
std::stack<glm::vec3> aStarPath;
glm::vec3 agentTarget;
glm::vec3 agentDirection;

std::vector<std::vector<int>> map =	{ 
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

int main()
{
	int errorGraphics = myGraphics.Init();			// Launch window and graphics context
	if (errorGraphics) return 0;					// Close if something went wrong...

	startup();										// Setup all necessary information for startup (aka. load texture, shaders, models, etc).



	// MAIN LOOP run until the window is closed
	while (!quit) {

		// Update the camera transform based on interactive inputs or by following a predifined path.
		updateCamera();

		// Update position, orientations and any other relevant visual state of any dynamic elements in the scene.
		updateSceneElements();

		// Render a still frame into an off-screen frame buffer known as the backbuffer.
		renderScene();

		// Swap the back buffer with the front buffer, making the most recently rendered image visible on-screen.
		glfwSwapBuffers(myGraphics.window);        // swap buffers (avoid flickering and tearing)

	}

	myGraphics.endProgram();            // Close and clean everything up...

   // cout << "\nPress any key to continue...\n";
   // cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void startup() {
	// Keep track of the running time
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime;                        // start delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Callback graphics and key update functions - declared in main to avoid scoping complexity.
	// More information here : https://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowSizeCallback(myGraphics.window, onResizeCallback);            // Set callback for resize
	glfwSetKeyCallback(myGraphics.window, onKeyCallback);                    // Set Callback for keys
	glfwSetMouseButtonCallback(myGraphics.window, onMouseButtonCallback);    // Set callback for mouse click
	glfwSetCursorPosCallback(myGraphics.window, onMouseMoveCallback);        // Set callback for mouse move
	glfwSetScrollCallback(myGraphics.window, onMouseWheelCallback);            // Set callback for mouse wheel.

	// Calculate proj_matrix for the first time.
	myGraphics.aspect = (float)myGraphics.windowWidth / (float)myGraphics.windowHeight;
	myGraphics.proj_matrix = glm::perspective(glm::radians(50.0f), myGraphics.aspect, 0.1f, 1000.0f);
	
	ps.Init();
	
	//TODO remove
	myGraphics.cameraPosition = glm::vec3(12.7f, 11.0f, 12.0f);
	myGraphics.cameraYaw = -138.0f;
	myGraphics.cameraPitch = -53.0f;

	// create vectors for instanced objects
	int index = 0;
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			floorPositions[index++] = glm::vec3(j,0.0f,i);
			if (map[i][j] == 1)
			{
				wallPositions.push_back(glm::vec3(j, 0.5f, i));
			}
		}
	}

	agentPosition = glm::vec3(7.0f, 0.5f, 7.0f);

	goalArrowPosition = glm::vec3(6.0f, 1.0f, 4.0f);



	test.Load();
	test.fillColor = glm::vec4(1.0f, 0.4f, 0.3f, 1.0f);
	// Load Geometry AI agent
	mySphere.Load();
	mySphere.fillColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

	myArrow.Load();
	myArrow.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	myArrow.lineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	// Load Geometry walls
	myWall.LoadInstanced(&wallPositions[0], wallPositions.size());
	myWall.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	
	// Load Geometry floor
	myFloor.LoadInstanced(&floorPositions[0],WIDTH*HEIGHT);
	myFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
	//myFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again

	// Optimised Graphics
	myGraphics.SetOptimisations();        // Cull and depth testing

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

	// IMPORTANT PART
	// Calculate my view matrix using the lookAt helper function
	if (mouseEnabled) {
		myGraphics.viewMatrix = glm::lookAt(myGraphics.cameraPosition,			// eye
			myGraphics.cameraPosition + myGraphics.cameraFront,					// centre
			myGraphics.cameraUp);												// up
	}
}


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

void updateSceneElements() {

	glfwPollEvents();                                // poll callbacks

	// Calculate frame time/period -- used for all (physics, animation, logic, etc).
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime - lastTime;                // Calculate delta time
	lastTime = currentTime;                            // Save for next frame calculations.


	//ps.Update(deltaTime);
	// Do not forget your ( T * R * S ) http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/

	// calculate Sphere movement
	updateAgentPosition();
	if (agentTarget != glm::vec3(0))
	{
		moveAgentToTarget();
	}
	
	test.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(6.0f, 2.0f, 6.0f)) *
		glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) *
		glm::mat4(1.0f);
	test.proj_matrix = myGraphics.proj_matrix;
	
	glm::mat4 mv_matrix_sphere =
		glm::translate(agentPosition) *
		glm::mat4(1.0f);
	mySphere.mv_matrix = myGraphics.viewMatrix * mv_matrix_sphere;
	mySphere.proj_matrix = myGraphics.proj_matrix;

	//Calculate Arrow for Astar goal
	glm::mat4 mv_matrix_arrow =
		glm::translate(goalArrowPosition) *
		glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
		glm::scale(glm::vec3(1.5f, 1.0f, 1.5f)) *
		glm::mat4(1.0f);
	myArrow.mv_matrix = myGraphics.viewMatrix * mv_matrix_arrow;
	myArrow.proj_matrix = myGraphics.proj_matrix;

	// Calculate floor position and resize
	myFloor.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::scale(glm::vec3(1.0f, 0.001f, 1.0f)) *
		glm::mat4(1.0f);
	myFloor.proj_matrix = myGraphics.proj_matrix;

	//Calculate wall position
	myWall.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::mat4(1.0f);
	myWall.proj_matrix = myGraphics.proj_matrix;

	t += 0.01f; // increment movement variable


	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; // If quit by pressing x on window.

}

void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();

	// Draw objects in screen
	test.Draw();
	myFloor.DrawInstanced(WIDTH*HEIGHT);
	myWall.DrawInstanced(wallPositions.size());
	mySphere.Draw();
	myArrow.Draw();
	//ps.Render(myGraphics.viewMatrix, myGraphics.proj_matrix);
}


// CallBack functions low level functionality.
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

	if (keyStatus[GLFW_KEY_C])
	{
		std::cout << "CAM position: " << glm::to_string(myGraphics.cameraPosition) << std::endl;
		std::cout << "CAM yaw: " << myGraphics.cameraYaw << std::endl;
		std::cout << "CAM pitch: " << myGraphics.cameraPitch << std::endl;
	}


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
			std::cout<< "agent position" << glm::to_string(agentPosition) << std::endl;
			std::cout<< "goal position" << glm::to_string(goalArrowPosition) << std::endl;
			aStarPath = Astar::getInstance()->path(map,agentPosition, goalArrowPosition);
			/*std::cout << "printing path" << std::endl;
			while(!aStarPath.empty())
			{
				std::cout << glm::to_string(aStarPath.top()) << std::endl;
				aStarPath.pop();
			}*/
		}
	}
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
