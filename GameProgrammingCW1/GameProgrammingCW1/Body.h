#ifndef BODY_H
#define BODY_H

#include "Shapes.h" 

enum class Direction { Left, Right, Up, Down, Idle };
class Body
{
public:
	Body(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	~Body();


	void Update(float time);
	

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
	Direction	direction = Direction::Idle;
	glm::vec3				velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3				acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3				addedForce = glm::vec3(0.0f, 0.0f, 0.0f);
	float					topSpeed = 5.0f;
	float					debugSpeed = 1;
	bool					isColliding = false;
	bool					onGround = false;

	bool					isBoid = false;
	glm::vec3				boidVelocity;

	bool					isStatic = false;

private:
	void CalculatePhysics(float deltatime);
	void CalculateDirection();
	void CalculateSides();
};


#endif // !BODY_H
