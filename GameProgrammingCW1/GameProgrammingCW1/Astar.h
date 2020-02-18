#ifndef ASTAR_H
#define ASTAR_H

#include <glm/glm.hpp>

#include <vector>
#include <queue>
#include <set>

struct Point
{
	int x;
	int y;

	Point()
	{
		this->x = 0;
		this->y = 0;
	}

	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	std::vector<Point> getNeighbours(int directions)
	{
		//return north-south-west-east if directions == 4
		std::vector<Point> neighbours;
		neighbours.push_back(Point(x, y - 1));	//NORTH
		neighbours.push_back(Point(x, y + 1));	//SOUTH
		neighbours.push_back(Point(x - 1, y));	//WEST
		neighbours.push_back(Point(x + 1, y));	//EAST
		//if direction == 8 add corness n-w n-e s-e s-w
		if (directions == 8)
		{
			neighbours.push_back(Point(x - 1, y - 1));	//NORTH-WEST
			neighbours.push_back(Point(x + 1, y - 1));	//NORTH-EAST
			neighbours.push_back(Point(x + 1, y + 1));	//SUD-EAST
			neighbours.push_back(Point(x - 1, y + 1));	//SUD-WEAST
		}
		return neighbours;
	}

	bool operator<(const Point& point) const {
		if (x < point.x) return true;
		if (x > point.x) return false;
		return y < point.y;
	}
	bool operator>(const Point& point) const {
		if (x > point.x) return true;
		if (x < point.x) return false;
		return y > point.y;
	}
	bool operator==(const Point& point) const {
		return x == point.x && y == point.y;
	}

	bool operator!=(const Point& point) const {
		return x != point.x || y != point.y;
	}

	Point operator+(const Point& point) const {
		return  Point(x + point.x, y + point.y);
	}
};

struct Cell
{
	Point parent;
	Point position;
	float f;

	Cell(Point pos)
	{
		this->position = pos;
		this->parent = this->position;
		f = 0;
	}

	Cell(Point pos, Point parent)
	{
		this->position = pos;
		this->parent = parent;
		f = 0;
	}

	friend bool operator<(const Cell& lhs, const Cell& rhs)
	{
		return lhs.f > rhs.f;
	}

	friend bool operator==(const Cell& lhs, const Cell& rhs)
	{
		return lhs.position > rhs.position;
	}
};

class Astar
{
public:
	static Astar* getInstance();
	std::vector<glm::vec3> path(std::vector<std::vector<int>>& map, glm::vec3 start, glm::vec3 goal);
	Point vec3ToPoint(glm::vec3 vector);
	glm::vec3 pointToVec3(Point point);

	void setParams(int width, int height, int directions);

private:
	static Astar* instance;
	Astar();

	int width = 0; // columns
	int height = 0; // rows
	int directions = 4;

	float manhattanHeuristic(Point current, Point goal); // to use with 4 directions
	float diagonalHeuristic(Point current, Point goal); // to use with 8 directions
	float distanceFromStart(Point current, Point start);

	bool isWall(Point destination, std::vector<std::vector<int>>& map);
	bool isGoal(Point destination, Point goal);
	bool isValidPoint(Point destination);
};

#endif // !ASTAR_H
