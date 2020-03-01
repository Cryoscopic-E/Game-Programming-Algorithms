#pragma once

#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shapes {

public:
	Shapes();
	~Shapes();

	void Load();
	void LoadInstanced(glm::mat4* models, const int num);
	void LoadParticles(glm::mat4* models, const int num);
	void Draw();
	void DrawInstanced(const int numInstances);
	void UpdateModelBuffer(glm::mat4* models, const int num);
	void  checkErrorShader(GLuint shader);

	vector<GLfloat> vertexPositions;

	GLuint          program;
	GLuint          vao;
	GLuint			texture;
	GLuint          vertex_buffer;
	GLuint			model_buffer;
	GLint           mv_location;
	GLint			view_location;
	GLint           proj_location;
	GLint           color_location;
	GLint			texture_location;
	glm::mat4		proj_matrix = glm::mat4(1.0f);
	glm::mat4		mv_matrix = glm::mat4(1.0f);
	glm::mat4		view_matrix = glm::mat4(1.0f);

	glm::vec4		fillColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	glm::vec4		lineColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
	float			lineWidth = 2.0f;

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

class Quad : public Shapes {
public:
	Quad();
	~Quad();
};


class Line : public Shapes {
public:
	Line();
	~Line();
};