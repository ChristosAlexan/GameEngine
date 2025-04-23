#pragma once
#include<string>
#include <fstream>
#include "Entity.h"
#include "Light.h"
#include "CollisionObject.h"

class SaveSystem
{
public:
	SaveSystem();
	void Save(std::vector<std::shared_ptr<Entity>>& entities, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, std::vector<CollisionObject>& collisionObject);
	void Load();
	void LoadEntityData(std::vector<std::shared_ptr<Entity>>& entities);
	void LoadLightData(std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights);
	void LoadCollisionObjectData(std::vector<CollisionObject>& collisionObject);

public:
	unsigned int entitiesCount, lightsCount, pointLightsCount, collisionObjectCount;
};

