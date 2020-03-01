#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;
	float lifeTime;
	float rotation;
	float scale;
};

class ParticleSystem
{
public:
	void Init();
	void Update(float delta);
	void UpdateBuffers();
	void Render(glm::mat4 viewMatrix, glm::mat4 projMatrix);

	ParticleSystem(unsigned int mp, glm::vec3 start);
	~ParticleSystem();
private:

	glm::vec3 startPosition;

	const float GRAVITY = -10.0f;
	const float PARTICLE_MASS = 1.0f;
	const float PARTICLE_SPEED = 3.0f;

	std::vector<Particle> particles;

	static const GLfloat quad_buffer_data[];

	GLuint vao;
	GLuint vbo_vertex;
	GLuint vbo_position;
	GLuint vbo_color;

	GLuint shaderProgram;

	GLuint cameraRight;     
	GLuint cameraUp;        
	GLuint vpMatrix;
	GLuint texture;

	unsigned int maxParticles;
	unsigned int particleTexture;

	GLfloat* position_data;
	GLubyte* color_data;

	void setParticlesBuffer(unsigned int &currentParticle, unsigned int &particleCount);

	void loadShader();
	void checkErrorShader(GLuint shader);
	void generateParticles();
};

#endif // !PARTICLE_SYSTEM_HPP
