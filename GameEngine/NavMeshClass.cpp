#include "NavMeshClass.h"
#include <list>

NavMeshClass::NavMeshClass()
{
	startNode = nullptr;
	endNode = nullptr;
	hasInit = false;
	timer.Start();
}

bool NavMeshClass::CalculatePath(float& dt, Entity* start, Entity* end, AIController& controller, GridClass& grid, float& gravity)
{
	if (!end || !start)
	{
		OutputDebugStringA(("LINE: " + std::to_string(__LINE__) + "\t").c_str());
		return false;
	}

	if (!start->physicsComponent.aActor || !end->physicsComponent.aActor ||
		!end->physicsComponent.isCharacter || end->bFlagForDeletion || start->bFlagForDeletion)
	{
		OutputDebugStringA(("LINE: " + std::to_string(__LINE__) + "\t").c_str());
		return false;
	}

	start->physicsComponent.trans = start->physicsComponent.aActor->getGlobalPose();
	controller.v1 = start->physicsComponent.trans.p;

	end->physicsComponent.trans = end->physicsComponent.aActor->getGlobalPose();
	controller.v2 = end->physicsComponent.trans.p;


	if (!hasInit || (controller.v2.x < endNode->pos.x - grid.acceptedRadius || controller.v2.x > endNode->pos.x + grid.acceptedRadius ||
		controller.v2.z < endNode->pos.z - grid.acceptedRadius || controller.v2.z > endNode->pos.z + grid.acceptedRadius))
	{
		hasInit = true;
		for (int i = 0; i < validNodes.size(); ++i)
		{
			// Initialize A* values
			validNodes[i].gCost = 0.0f;
			validNodes[i].hCost = 0.0f;
			validNodes[i].parent = nullptr;
			validNodes[i].isInCloseList = false;
			validNodes[i].isInOpenList = false;

			if (controller.v1.x >= validNodes[i].pos.x - grid.acceptedRadius && controller.v1.x <= validNodes[i].pos.x + grid.acceptedRadius &&
				controller.v1.z >= validNodes[i].pos.z - grid.acceptedRadius && controller.v1.z <= validNodes[i].pos.z + grid.acceptedRadius)
			{
				//Find start node
				startNode = &validNodes[i];
			}

			if (controller.v2.x >= validNodes[i].pos.x - grid.acceptedRadius && controller.v2.x <= validNodes[i].pos.x + grid.acceptedRadius &&
				controller.v2.z >= validNodes[i].pos.z - grid.acceptedRadius && controller.v2.z <= validNodes[i].pos.z + grid.acceptedRadius)
			{
				//Find destination node
				endNode = &validNodes[i];
			}
		}

		if (!startNode || !endNode)
		{
			OutputDebugStringA(("LINE: " + std::to_string(__LINE__) + "\t").c_str());
			return false;
		}
			

		solve_async = std::async(std::launch::async, &NavMeshClass::Solve_AStar, this, std::ref(dt), start, end);

		if (!solve_async.valid())
		{
			OutputDebugStringA(("LINE: " + std::to_string(__LINE__) + "\t").c_str());
			return false;
		}
	}
	
	if (solve_async._Is_ready()) {
		try {
			//Wait for the asynchronous task to finish
			solve_async.get();
		}
		catch (const std::exception& e) {
			OutputDebugStringA(("Exception caught: " + std::string(e.what()) + " at LINE: " + std::to_string(__LINE__) + "\t").c_str());
			return false;
		}
	}
	
	for (int i = start->locations.size() - 1; i >= 0; --i)
	{
		if (controller.v1.x >= start->locations[i].x - grid.acceptedRadius && controller.v1.x <= start->locations[i].x + grid.acceptedRadius &&
			controller.v1.z >= start->locations[i].z - grid.acceptedRadius && controller.v1.z <= start->locations[i].z + grid.acceptedRadius)
		{
			if (start->m_index != i)
				start->m_index = i;
		}
	}

	if (!start->locations.empty())
	{
		start->locToMove = start->locations[start->m_index];
	}

	controller.MoveTo(dt, start, end, gravity);

	return true;
}

void NavMeshClass::Solve_AStar(float& dt, Entity* start, Entity* end)
{
	std::list<NodeClass*> openList, closeList;
	openList.push_back(startNode);

	while (!openList.empty())
	{
		NodeClass* currentNode = openList.front();
		openList.sort([](const NodeClass* lhs, const NodeClass* rhs) {return lhs->fCost < rhs->fCost; });
		currentNode->fCost = currentNode->gCost + currentNode->hCost;

		for (auto _openNode : openList)
		{
			_openNode->fCost = _openNode->gCost + _openNode->hCost;
			if (_openNode->fCost < currentNode->fCost || _openNode->fCost == currentNode->fCost && _openNode->hCost < currentNode->hCost)
				currentNode = _openNode;
		}

		currentNode->isInOpenList = false;
		currentNode->isInCloseList = true;
		openList.remove(currentNode);
		closeList.push_back(currentNode);
	
		if (currentNode == endNode)
		{
			RetracePath(*startNode, *endNode, start);
			start->m_index = start->locations.size() - 1;
			return;
		}
			

		for (auto neighbour : currentNode->neighbours)
		{
			if (!neighbour->isValidPath || neighbour->isInCloseList)
				continue;

			float newCost = currentNode->gCost + Vec3Distance(*currentNode, *neighbour);
			if (newCost < neighbour->gCost || !neighbour->isInOpenList)
			{
				neighbour->gCost = newCost;
				neighbour->hCost = Vec3Distance(*neighbour, *endNode);
				neighbour->fCost = neighbour->gCost + neighbour->hCost;
				neighbour->parent = currentNode;
				openList.push_back(neighbour);
			}
		}
	}
}
float NavMeshClass::Vec3Distance(NodeClass& node1, NodeClass& node2)
{
	physx::PxVec3 a = physx::PxVec3(node1.pos.x, node1.pos.y, node1.pos.z);
	physx::PxVec3 b = physx::PxVec3(node2.pos.x, node2.pos.y, node2.pos.z);
	return sqrtf((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
}

float NavMeshClass::Vec3Distance(physx::PxVec3& location, NodeClass& node2)
{
	physx::PxVec3 a = physx::PxVec3(location.x, location.y, location.z);
	physx::PxVec3 b = physx::PxVec3(node2.pos.x, node2.pos.y, node2.pos.z);
	return sqrtf((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
}

void NavMeshClass::RetracePath(NodeClass& _startNode, NodeClass& endNode, Entity* start)
{
	start->locations.clear();
	std::list<NodeClass*> path;
	NodeClass* currentNode = &endNode;

	while (currentNode != startNode)
	{
		path.push_back(currentNode);
		currentNode = currentNode->parent;
	}
	for (auto _node : path)
	{
		start->locations.push_back(physx::PxVec3(_node->pos.x, _node->pos.y, _node->pos.z));
	}
}
