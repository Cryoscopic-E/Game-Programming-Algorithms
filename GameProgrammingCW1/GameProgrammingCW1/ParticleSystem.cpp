#include "ParticleSystem.h"
#include <time.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const GLfloat ParticleSystem::quad_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
};

void ParticleSystem::Init()
{
	//compile shader program
	loadShader();
	//fill particles array
	generateParticles();

	//texture
	int width, height, channels;
	unsigned char* data = stbi_load("texture.png", &width, &height, &channels, 0);
	glGenTextures(1, &particleTexture);
	glBindTexture(GL_TEXTURE_2D, particleTexture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "failed generating texture" << std::endl;
	}
	stbi_image_free(data);

	//uniforms
	cameraUp = glGetUniformLocation(shaderProgram, "cameraUp");
	cameraRight = glGetUniformLocation(shaderProgram, "cameraRight");
	vpMatrix = glGetUniformLocation(shaderProgram, "vp");
	texture = glGetUniformLocation(shaderProgram, "billBoardTexture");

	// GENERATE BUFFERS

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//vertex
	glGenBuffers(1, &vbo_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_buffer_data), quad_buffer_data, GL_STATIC_DRAW);

	//positions
	glGenBuffers(1, &vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//colors
	glGenBuffers(1, &vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	glBindVertexArray(0);
	
	glLinkProgram(0);

}

void ParticleSystem::Update(float delta)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < maxParticles; i++)
	{
		setParticlesBuffer(i, count);
		count++;
	}
	UpdateBuffers();
}

void ParticleSystem::UpdateBuffers()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); //orphaning, allocating faster than sync
	glBufferSubData(GL_ARRAY_BUFFER, 0, maxParticles * sizeof(GLfloat) * 4, position_data);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, maxParticles * sizeof(GLubyte) * 4, color_data);

}

void ParticleSystem::Render(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	//set uniforms
	glUniform3f(cameraRight, viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	glUniform3f(cameraUp, viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
	glm::mat4 vp = viewMatrix * projMatrix;
	glUniformMatrix4fv(vpMatrix, 1, GL_FALSE, &vp[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleTexture);
	glUniform1i(texture, 0);
	

	// 1st attrib buffer vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attrib buffer positions of particle
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3th attrib buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);

	// draw particles
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, maxParticles);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);
}

ParticleSystem::ParticleSystem(unsigned int mp, glm::vec3 start)
{
	maxParticles = mp;
	startPosition = start;
	position_data = new GLfloat[maxParticles * 4];
	color_data = new GLubyte[maxParticles * 4];
}

ParticleSystem::~ParticleSystem()
{

	glDeleteBuffers(1, &vbo_color);
	glDeleteBuffers(1, &vbo_position);
	glDeleteBuffers(1, &vbo_vertex);
	delete position_data;
	delete color_data;
}

void ParticleSystem::setParticlesBuffer(unsigned int &currentParticle, unsigned int &particleCount)
{
	Particle& p = particles[currentParticle];
	
	position_data[4 * particleCount + 0] = p.position.x;
	position_data[4 * particleCount + 1] = p.position.y;
	position_data[4 * particleCount + 2] = p.position.z;
	position_data[4 * particleCount + 3] = p.scale;

	color_data[4 * particleCount + 0] = p.color.r;
	color_data[4 * particleCount + 1] = p.color.g;
	color_data[4 * particleCount + 2] = p.color.b;
	color_data[4 * particleCount + 3] = p.color.a;
}

void ParticleSystem::loadShader()
{

	const char* vs_source[] = { R"(
		#version 330 core

		layout (location = 0) in vec3 vert;
		layout (location = 1) in vec4 position;
		layout (location = 2) in vec4 color;
		
		uniform vec3 cameraUp;
		uniform vec3 cameraRight;
		uniform mat4 vp;

		out vec4 quadColor;
		out vec2 uv;

		void main()
		{			
			vec3 vertexPosition = 
				position.xyz
				+ cameraRight * vert.x * position.w
				+ cameraUp * vert.y * position.w;

			
			gl_Position = vp * vec4(vertexPosition, 1.0f);
			uv = vert.xy + vec2(0.5, 0.5);
			quadColor = color;
		}
)" };


	const char* fs_source[] = { R"(
		#version 330 core

		in vec4 quadColor;
		in vec2 uv;
		out vec4 color;
		
		uniform sampler2D billBoardTexture;		


		void main()
		{
			color = texture2D( billBoardTexture, uv ) *   quadColor;
		}
)" };

	shaderProgram = glCreateProgram();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);

	glLinkProgram(shaderProgram);
}

void ParticleSystem::checkErrorShader(GLuint shader) {
	// Get log length
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		std::cout << "--------------Shader compilation error-------------\n";
		std::cout << errorLog.data();
	}
}

void ParticleSystem::generateParticles()
{
	for (unsigned int i = 0; i < maxParticles; i++)
	{
		Particle newParticle;
		newParticle.position = startPosition;
		srand(time(0));
		newParticle.velocity.x = ((((float)rand()/(float)RAND_MAX) * 2.0f) -1.0f) * PARTICLE_SPEED; // between -1 and 1 float
		
		newParticle.velocity.y = ((float)rand()/(float)RAND_MAX) * PARTICLE_SPEED; // only up not through floor
		
		newParticle.velocity.z = ((((float)rand()/(float) RAND_MAX) * 2.0f) - 1.0f) * PARTICLE_SPEED;  // between -1 and 1 float
		
		newParticle.scale = 100.0f; // between 1 and 1.5 float

		newParticle.color.r = 1.0f;
		newParticle.color.g = 1.0f;
		newParticle.color.b = 1.0f;
		newParticle.color.a = 1.0f;

		particles.push_back(newParticle);
	}
}
