#ifndef PLAYER_H
#define PLAYER_H

#include "Shapes.h"
#include "Body.h"

class Player
{
public:
	Player();
	~Player();

	Cube avatar;
	Body* physicBody;
};
#endif // !PLAYER_H

