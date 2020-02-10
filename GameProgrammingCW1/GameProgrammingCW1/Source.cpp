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
#include "graphics.h"
#include "shapes.h"
#include "directions.h"

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

//MADE FUNCTIONS
void UpdatePlayerInput();
bool CheckCollision(Shapes& obj1, Shapes& obj2);

// VARIABLES
bool        quit = false;
float       deltaTime = 0.0f;    // Keep track of time per frame.
float       lastTime = 0.0f;    // variable to keep overall time.
bool        keyStatus[1024];    // Hold key status.
bool		mouseEnabled = true; // keep track of mouse toggle.

//Grid Elements
vector<Cube> gameGrid;
int			gridSize = 10;

//Boids 
vector<Sphere> boidList;
int			amountOfBoids = 30;
glm::vec3 GetBoidsCenter();
void BoidRule1();
void BoidRule2();
void BoidRule3();

//CameraLock
bool		lockCamera = true;

// MAIN GRAPHICS OBJECT
Graphics    myGraphics;        // Runing all the graphics in this object

// DEMO OBJECTS
Cube        player;
Cube        myFloor;
Arrow       arrowZ;


// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation


int main()
{
	int errorGraphics = myGraphics.Init();			// Launch window and graphics context
	if (errorGraphics) return 0;					// Close if something went wrong...

	startup();										// Setup all necessary information for startup (aka. load texture, shaders, models, etc).



	// MAIN LOOP run until the window is closed
	while (!quit) {

		// Update the camera transform based on interactive inputs or by following a predifined path.
		updateCamera();
		
		// Update player inputs
		UpdatePlayerInput();

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

	// Load Geometry examples
	player.Load();
	player.position = glm::vec3(5.0f, 0.5f, 5.0f);
	player.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	arrowZ.Load();
	arrowZ.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); 
	arrowZ.lineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	myFloor.Load();
	myFloor.position = glm::vec3(0.0f, 0.0f, 0.0f);
	myFloor.scale = glm::vec3(1000.0f, 0.001f, 1000.0f);
	myFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
	myFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again

	for (int i = 0; i < amountOfBoids; i++) 
	{
		Sphere tempBoid;
		tempBoid.debugSpeed = 1;
		tempBoid.Load();
		tempBoid.isBoid = true;
		tempBoid.topSpeed = 0.2f;
		tempBoid.position = glm::vec3(rand() % 10 + 1, 1.0f, rand() % 10 + 1);
		tempBoid.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); 

		boidList.push_back(tempBoid);
	}

	for (int i = 0; i < gridSize; i++) 
	{
		Cube tempCube;
		tempCube.Load();
		tempCube.position = glm::vec3(i, 0.5f, 0.0f);
		gameGrid.push_back(tempCube);

		if (i == 0 || i == gridSize - 1)
		{
			for (int j = 0; j < gridSize; j++)
			{
				Cube tempCube;
				tempCube.Load();
				tempCube.position = glm::vec3(i, 0.5f, j);
				gameGrid.push_back(tempCube);
				
				if (i == 0 && j == gridSize - 1) 
				{
					for (int k = 0; k < gridSize; k++) 
					{
						Cube tempCube;
						tempCube.Load();
						tempCube.position = glm::vec3(k, 0.5f, j);
						gameGrid.push_back(tempCube);
					}
				}
			}
		}

	}

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

	//TODO:: CHANGE HARDCODED CAMERA
	if (lockCamera) {
		myGraphics.cameraPitch = -90;
		myGraphics.cameraYaw = 90;
		myGraphics.cameraPosition = glm::vec3(4, 14, 4);
	}

}
void UpdatePlayerInput() 
{
	if (keyStatus[GLFW_KEY_UP])
	{
		player.velocity.x = 0.0f;
		if (player.velocity.z < 0) player.velocity.z = 0.0f;
		player.acceleration.z = 0.1f;
	}
	else if (keyStatus[GLFW_KEY_DOWN]) {
		player.velocity.x = 0.0f;
		if (player.velocity.z > 0) player.velocity.z = 0.0f;
		player.acceleration.z = -0.1f;
	}
	else if (keyStatus[GLFW_KEY_RIGHT]) {
		player.velocity.z = 0.0f;
		if (player.velocity.x > 0) player.velocity.x = 0.0f;
		player.acceleration.x = -0.1f;
	}
	else if (keyStatus[GLFW_KEY_LEFT]) {
		player.velocity.z = 0.0f;
		if (player.velocity.x < 0) player.velocity.x = 0.0f;
		player.acceleration.x = 0.1f;
	}
	else {
		player.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		player.acceleration.x = 0.0f;
		player.acceleration.z = 0.0f;
	}

	if (keyStatus[GLFW_KEY_C]) 
	{
		if (lockCamera) {
			lockCamera = false;
		}
		else {
			lockCamera = true;
		}
	}
	
}
void updateSceneElements() {

	glfwPollEvents();                                // poll callbacks

	// Calculate frame time/period -- used for all (physics, animation, logic, etc).
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime - lastTime;                // Calculate delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Do not forget your ( T * R * S ) http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
	
	for (int i = 0; i < gameGrid.size(); i++)
	{
		gameGrid[i].Update(myGraphics, deltaTime);
	}

	BoidRule1();
	BoidRule2();
	BoidRule3();

	for (int i = 0; i < boidList.size(); i++)
	{
		boidList[i].velocity += boidList[i].boidVelocity;
		boidList[i].Update(myGraphics, deltaTime);
		printf("VELOCITY %f %f %f \n", boidList[i].velocity.x, boidList[i].velocity.y, boidList[i].velocity.z);
		boidList[i].boidVelocity = glm::vec3(0,0,0);
	}

	player.Update(myGraphics, deltaTime);
	myFloor.Update(myGraphics, deltaTime);
	arrowZ.Update(myGraphics, deltaTime);

	for (int i = 0; i < gameGrid.size(); i++) {
		CheckCollision(player, gameGrid[i]);
	}

	for (int i = 0; i < boidList.size(); i++) {
		for (int j = 0; j < gameGrid.size(); j++) {
			CheckCollision(boidList[i], gameGrid[i]);
		}
		//for (int j = 0; j < boidList.size(); j++)
		//{
		//	//if (i != j)
		//		//CheckCollision(boidList[i], boidList[j]);			
		//}

	}

	t += 0.01f; // increment movement variable


	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; // If quit by pressing x on window.

}

bool CheckCollision(Shapes& obj1, Shapes& obj2)
{
	if (obj1.left < obj2.right && obj1.right > obj2.left && obj1.up > obj2.down && obj1.down <obj2.up)
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
		else if(obj1.isBoid && !obj2.isBoid)
		{
			obj1.addedForce = (obj1.velocity * glm::vec3(1.5f, 1.5f, 1.5f));// *glm::vec3(-1, -1, -1);
			return true;
		}

		obj1.isColliding = true;
		if (obj1.direction == Directions::Direction::Left && obj2.direction == Directions::Direction::Idle)
		{
			obj1.addedForce = glm::vec3(-5, 0, 0);
			printf("COLLIDING LEFT\n");

		}
		else if (obj1.direction == Directions::Direction::Right && obj2.direction == Directions::Direction::Idle) 
		{
			obj1.addedForce = glm::vec3(5, 0, 0);
			printf("COLLIDING RIGHT\n");

		}else if (obj1.direction == Directions::Direction::Up && obj2.direction == Directions::Direction::Idle)
		{
			obj1.addedForce = glm::vec3(0, 0, -5);
			printf("COLLIDING UP\n");
		}
		else if (obj1.direction == Directions::Direction::Down && obj2.direction == Directions::Direction::Idle)
		{
			obj1.addedForce = glm::vec3(0, 0, 5);
			printf("COLLIDING DOWN \n");
		}
		//obj1.Update(myGraphics, deltaTime);
		//printf("COLLIDING BOYYY");
		return true;
	}
	obj1.isColliding = false;
	obj2.isColliding = false;
	return false;
}

void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();

	// Draw objects in screen
	for (int i = 0; i < boidList.size(); i++)
	{
		boidList[i].Draw();
	}
	
	for (int i = 0; i < gameGrid.size(); i++)
	{
		gameGrid[i].Draw();
	}
	myFloor.Draw();
	player.Draw();
	//arrowZ.Draw();

}

glm::vec3 GetBoidsCenter() 
{
	glm::vec3 temp = glm::vec3(0,0,0);
	for (int i = 0; i < boidList.size(); i++) 
	{
		temp += boidList[i].position;
	}

	glm::vec3 temp2 = glm::vec3(temp.x / boidList.size(), temp.y / boidList.size(), temp.z / boidList.size());
	return temp2;
}

void BoidRule1()
{
	for (int i = 0; i < boidList.size(); i++)
	{
		boidList[i].boidVelocity += (GetBoidsCenter() - boidList[i].position) / glm::vec3(100,100,100);
		boidList[i].boidVelocity += (player.position - boidList[i].position) / glm::vec3(8,8,8);
	}

}

void BoidRule2() 
{
	for (int i = 0; i < boidList.size(); i++) 
	{
		if (boidList[i].position.y <1) 
		{
			boidList[i].boidVelocity += 1;
		}

		for (int j = 0; j < boidList.size(); j++) 
		{
			if (i != j)
			{
				if (glm::distance(boidList[i].position, boidList[j].position) < 1)
				{
					//printf("POS1 %f %f POS2 %f %f \n", boidList[i].position.x, boidList[i].position.z, boidList[j].position.x, boidList[j].position.z);
					boidList[i].boidVelocity += boidList[i].position - boidList[j].position;
				}
			}
		}
		
		for (int j = 0; j < gameGrid.size(); j++) 
		{
			if (glm::distance(boidList[i].position, gameGrid[j].position) < 1.2f)
			{
				boidList[i].boidVelocity += boidList[i].position - gameGrid[j].position;
			}
		}
		
	}
}

void BoidRule3() 
{
	glm::vec3 tempVel = glm::vec3(0, 0, 0);
	for (int i = 0; i < boidList.size(); i++) 
	{
		for (int j = 0; j < boidList.size(); j++)
		{
			tempVel += boidList[j].velocity;
		}

		tempVel = glm::vec3(tempVel.x / boidList.size(), tempVel.y / boidList.size(), tempVel.z / boidList.size());
		boidList[i].boidVelocity += tempVel / glm::vec3(8,8,8);
	}	
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
