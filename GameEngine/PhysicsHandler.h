#pragma once
#include <PhysX/PxPhysicsAPI.h>
#include"PxDefaultErrorCallback.h"
#include"PxDefaultAllocator.h"
#include"AppTimer.h"
#include<vector>
#include "Entity.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Camera.h"
#include "CollisionObject.h"
#include "GridClass.h"
#include "TpsController.h"
#include "DX11.h"
#include "PhysicsDebugDraw.h"
#include "RectShape.h"

#include <NvBlastTk.h>
class PhysicsHandler
{
public:
	PhysicsHandler();
	~PhysicsHandler();

	void Initialize(Camera& camera);
	void CreatePhysicsComponents(std::vector<std::shared_ptr<Entity>>& entities, std::vector<CollisionObject>& collisionObject);
	void MouseRayCast(std::vector<std::shared_ptr<Entity>>& entities, Camera& camera, Mouse& mouse, Keyboard& keyboard, int& width, int& height, int& selected_list_object);
	void FallCheck(Entity* character);
	void NavMeshRayCast(GridClass& grid, std::vector<std::shared_ptr<Entity>>& entities, std::vector<CollisionObject>& collisionObjects);
	void LineOfSightToPlayer(Entity* character, Entity* player);
	void PlayerFireRayTrace(TpsController& tpsPlayerController, Entity* player, std::vector<std::shared_ptr<Entity>>& entities, Camera& camera);
	void CrosshairRayTrace(TpsController& tpsPlayerController, Entity* player, std::vector<std::shared_ptr<Entity>>& entities, RectShape& crosshair, Camera& camera);
	bool advance(float& dt, float& fps, Camera& camera);
	void DrawDebugLine(DX11& gfx11, PhysicsDebugDraw& physicsDebugDraw, Camera& camera);
	void ShutDown();

public:
	physx::PxScene* aScene;
	physx::PxPhysics* mPhysics;
	physx::PxControllerManager* manager;
	bool isMouseHover;
	bool bRenderCollisionShape;
	bool bDebugLines;

	std::vector<physx::PxVec3> rayOriginOut;
	std::vector<physx::PxVec3> rayDestOut;
	
	float rayDist;
private:
	physx::PxFoundation* mFoundation;
	physx::PxPvd* mPvd;

	physx::PxDefaultCpuDispatcher* gDispatcher;


	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	AppTimer timer;
};

