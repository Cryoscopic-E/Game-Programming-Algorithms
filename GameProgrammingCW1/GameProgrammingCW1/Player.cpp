#include "Player.h"

Player::Player()
{
	avatar.Load();

	physicBody = new Body(glm::vec3(10.0f,0.5f,10.0f), glm::vec3(0.0f), glm::vec3(1.0f));
	
}

Player::~Player()
{
	delete physicBody;
}
