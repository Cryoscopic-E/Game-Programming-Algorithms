#include "Body.h"

Body::Body(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
	CalculateSides();
}

Body::~Body()
{
}

void Body::Update(float time)
{
	if (!isStatic)
	{
		CalculateSides();
		CalculatePhysics(time);
		CalculateDirection();
	}
}

void Body::CalculatePhysics(float deltatime)
{
	velocity += acceleration;

	if (velocity.x > topSpeed) {
		velocity.x = topSpeed;
	}
	else if (velocity.x < -topSpeed) {
		velocity.x = -topSpeed;
	}

	if (velocity.y > topSpeed) {
		velocity.y = topSpeed;
	}
	else if (velocity.y < -topSpeed) {
		velocity.y = -topSpeed;
	}

	if (velocity.z > topSpeed) {
		velocity.z = topSpeed;
	}
	else if (velocity.z < -topSpeed) {
		velocity.z = -topSpeed;
	}

	position += (velocity + addedForce) * deltatime;// *debugSpeed;
}

void Body::CalculateDirection()
{
	if (isColliding)
		return;
	
	addedForce = glm::vec3(0, 0, 0);
	if (velocity.x > 0) {
		direction = Direction::Left;
		if (!isBoid)printf(" LEFT \n");

	}
	else if (velocity.x < 0) {
		direction = Direction::Right;
		if (!isBoid)printf(" RIGHT \n");

	}
	else if (velocity.z > 0) {
		if (!isBoid)direction =Direction::Up;
		if (!isBoid)printf(" UP \n");

	}
	else if (velocity.z < 0) {
		direction = Direction::Down;
		if (!isBoid)printf(" DOWN \n");

	}
}

void Body::CalculateSides()
{
	left = position.x - (scale.x / 2);
	right = position.x + (scale.x / 2);
	down = position.z - (scale.z / 2);
	up = position.z + (scale.z / 2);
}
