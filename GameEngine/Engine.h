#pragma once
#include "WindowContainer.h"
#include "AppTimer.h"
#include "Renderer.h"
#include "PhysicsHandler.h"
#include <future>
#include "SaveSystem.h"
#include "CollisionObject.h"
#include "FpsController.h"
#include "TpsController.h"
#include "GridClass.h"
#include <thread>
#include "NavMeshClass.h"

#include <algorithm>
#include "Camera.h"
#include "Sky.h"
#include"SoundComponent.h"

class Engine : virtual WindowContainer
{
public:
	Engine();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update(int width, int height);
	void RenderFrame(float& dt, float& fps);
	void Shutdown();
private:
	void AddEntity(std::string& _inName, bool& isAnimated, bool& bConvertCordinates);
	void AddPhysicsComp(std::shared_ptr<Entity>& entity);
	void AddLight();
	void AddPointLight();
	void AddCollisionObject();
	void ObjectsHandler(float& dt);
	void AIHandler(float& dt);
	void CopyPasteEntity();
	void CopyPasteLight();
	void CopyPastePointLight();

	void PlayerLogic(float& dt);
	void GameSounds();

	void Async_FireRayCast();
protected:
	AppTimer timer;

private:
	Sky sky;
	std::vector<std::shared_ptr<Entity>> entities;

	std::vector<std::shared_ptr<Entity>> AIEntities;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Light>> pointlights;
	std::vector<CollisionObject> collisionObjects;
	std::vector<SoundComponent*> sounds;

	Camera camera;
	Renderer renderer;
	PhysicsHandler physicsHandler;

	int width, height;

	SaveSystem saveSystem;

	TpsController tpsPlayerController;
	FpsController fpsPlayerController;

	std::shared_ptr<Entity> player;

	AIController enemyController;
	std::vector<NavMeshClass> navMeshes;

	GridClass grid;
	//bool isNavMeshCreated = false;
	std::thread grid_test;

	bool initScene = true;
	bool bModelsLoaded = false;


	bool copyEntity = false;
	bool pasteEntity = false;
	bool bCanCopy = true;
	bool bCanPaste = true;


	std::unordered_map<std::string, Texture> globalTextureStorage;

	SoundComponent backGroundSound;
private:
		DirectX::XMFLOAT3 camTempPos, camTempRot;
		bool bProbesReady = true;


		std::future<void> async_rayCastNavMesh;
		std::vector< std::future<void>> async_navMesh;

		std::future<bool> async_aiCalculatePath;
		std::future<void> async_aiHandler, async_playerFallCheck;
		std::vector<std::future<void>> async_fallCheck, async_lineOfSightToPlayer;

		std::future<void> ai_async;

		int entityToCopy = -1;
};
