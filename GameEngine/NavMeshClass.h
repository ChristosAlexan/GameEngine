#pragma once

#include "GridClass.h"
#include<future>
#include "AIController.h"
#include <mutex>
class NavMeshClass
{
public:
	NavMeshClass();
	bool CalculatePath(float& dt, Entity* start, Entity* end, AIController& controller, GridClass& grid, float& gravity);
	void Solve_AStar(float& dt, Entity* start, Entity* end);
private:
	float Vec3Distance(NodeClass& node1, NodeClass& node2);
	float Vec3Distance(physx::PxVec3& location, NodeClass& node2);
	void RetracePath(NodeClass& _startNode, NodeClass& endNode, Entity* start);

	std::future<void> solve_async;
	NodeClass* startNode;
	NodeClass* endNode;
	AppTimer timer;
	bool hasInit;
public:

	std::vector<NodeClass> validNodes;
	bool recalculate;
};

