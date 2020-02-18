#include "Astar.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
Astar* Astar::instance = nullptr;

Astar::Astar()
{}

Astar* Astar::getInstance()
{
	if (instance == nullptr)
	{
		instance = new Astar();
	}
	return instance;
}

std::vector<glm::vec3> Astar::path(std::vector<std::vector<int>> & map, glm::vec3 start, glm::vec3 goal)
{
	std::vector<glm::vec3> path;

	Point startPoint = vec3ToPoint(start);
	Point goalPoint = vec3ToPoint(goal);

	// goal point not in map range
	if (!isValidPoint(goalPoint))
	{
		std::cout << "invalid: goal point out of map bounds" << std::endl;
		return std::vector<glm::vec3>();
	}
	// goal point not blocked by wall
	if (isWall(goalPoint, map))
	{
		std::cout << "invalid: goal point is wall" << std::endl;
		return std::vector<glm::vec3>();
	}
	// goal == start
	if (isGoal(startPoint, goalPoint))
	{
		std::cout << "goal point already reached!" << std::endl;
		return std::vector<glm::vec3>();
	}

	std::priority_queue<Cell> fringe;
	//std::set<Cell> closeSet;
	std::vector<Cell> visited;

	Cell startCell(startPoint);
	fringe.push(startCell);
	bool path_found = false;
	while (!fringe.empty() && !path_found)
	{
		Cell current = fringe.top();
		fringe.pop();

		//closeSet.insert(current.position);
		//if (std::find(visited.begin(), visited.end(), current) == visited.end())
		//{
			visited.push_back(current);
		//}
		

		std::vector<Point> neighbours = current.position.getNeighbours(this->directions);
		for (const Point& neighbour : neighbours)
		{
			Cell neighbourCell(neighbour);
			if (isValidPoint(neighbour)) //if generated point is in map bounds
			{
				if (neighbour == goalPoint) // next is neighbour
				{
					path_found = true;
					neighbourCell.parent = current.position;
					//visited.push_back(neighbourCell);
					std::cout<<"from " << glm::to_string(pointToVec3(current.position)) << std::endl;
					std::cout << "found" << std::endl;
					break;
				}
				else if (std::find(visited.begin(), visited.end(), current) == visited.end() && !isWall(neighbour,map))
				{
					neighbourCell.f = distanceFromStart(neighbour, startPoint) + ((directions == 8)? diagonalHeuristic(neighbour,goalPoint) : manhattanHeuristic(neighbour,goalPoint));
					neighbourCell.parent = current.position;
					fringe.push(neighbourCell);
				}
			}
		}
	}

	// RETURN EMPTY VECTOR IF NO PATH FOUND
	if (!path_found)
	{
		return std::vector<glm::vec3>();
	}
	// CONSTRUCT PATH WITH CLOSE SET REVERSE
	path.push_back(pointToVec3(goalPoint));
	Point lastparent;
	for (int i = visited.size() - 1; i > 0; --i)
	{
		if (lastparent == Point(0, 0) || lastparent == visited[i].position)
		{
			path.push_back(pointToVec3(visited[i].position));
			lastparent = visited[i].parent;
		}
	}
	std::reverse(path.begin(), path.end());
	return path;
}


Point Astar::vec3ToPoint(glm::vec3 vector)
{
	Point p;
	p.x = (int)vector.x;
	p.y = (int)vector.z;
	return p;
}

glm::vec3 Astar::pointToVec3(Point point)
{
	glm::vec3 v;
	v.x = (float)point.x;
	v.y = 0.5f;
	v.z = (float)point.y;
	return v;
}

void Astar::setParams(int width, int height, int directions)
{
	this->width = width;
	this->height = height;
	this->directions = directions;
}

// h value to use with 4 directions
float Astar::manhattanHeuristic(Point current, Point goal)
{
	return abs(current.x - goal.x) + abs(current.y - goal.y);
}
// h value to use with 8 directions
float Astar::diagonalHeuristic(Point current, Point goal)
{
	return fmaxf(abs(current.x - goal.x), abs(current.y - goal.y));
}

float Astar::distanceFromStart(Point current, Point start)
{
	if (directions == 4)
	{
		return  abs(current.x - start.x) + abs(current.y - start.y);
	}
	else
	{
		return fmaxf(abs(current.x - start.x), abs(current.y - start.y));
	}
}

bool Astar::isWall(Point destination, std::vector<std::vector<int>>& map)
{
	return map[destination.y][destination.x] == 1;
}

bool Astar::isGoal(Point destination, Point goal)
{
	return goal == destination;
}

bool Astar::isValidPoint(Point point)
{
	return point.x >= 0 && point.x < this->height && point.y >=0 && point.y < this->width;
}
