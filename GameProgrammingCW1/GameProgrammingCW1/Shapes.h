#pragma once

#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "graphics.h"
#include "directions.h"

class Shapes {

public:
	Shapes();
	~Shapes();

	void Load();
	void Draw();
	void checkErrorShader(GLuint shader);

	//ADDED Methods
	void Update(Graphics graphics, float time);
	void CalculatePhysics(float deltatime);
	void CalculateDirection();
	void CalculateSides();
	void DrawInstanced(int numBoids);

	vector<GLfloat> vertexPositions;

	GLuint					program;
	GLuint					vao;
	GLuint					buffer;
	GLint					mv_location;
	GLint					proj_location;
	GLint					color_location;
	glm::mat4				proj_matrix = glm::mat4(1.0f);
	glm::mat4				mv_matrix = glm::mat4(1.0f);

	glm::vec4				fillColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	glm::vec4				lineColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
	float					lineWidth = 2.0f;

	//Transform
	glm::vec3				position;
	glm::vec3				rotation;
	glm::vec3				scale;

	//BoundingBox
	float					left;
	float					right;
	float					down;
	float					up;

	//Movement
	Directions::Direction	direction = Directions::Direction::Idle;
	glm::vec3				velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3				acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3				addedForce = glm::vec3(0.0f, 0.0f, 0.0f);
	float					topSpeed = 5.0f;
	float					debugSpeed = 1;
	bool					isColliding = false;

	//Boids
	bool					isBoid;
	glm::vec3				boidVelocity;

protected:
	string rawData;			// Import obj file from Blender (note: no textures or UVs).
	void LoadObj();
};

class Cube : public Shapes {
public:
	Cube();
	~Cube();
};

class Sphere : public Shapes {
public:
	Sphere();
	~Sphere();
};

class Arrow : public Shapes {
public:
	Arrow();
	~Arrow();
};

class Cylinder : public Shapes {
public:
	Cylinder();
	~Cylinder();
};

class Line : public Shapes {
public:
	Line();
	~Line();
};

class Boid : public Shapes
{
public:
	Boid();
	~Boid();
};