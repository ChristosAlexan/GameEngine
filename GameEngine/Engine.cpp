#include "Engine.h"

using namespace DirectX;


Engine::Engine()
{
	//player = nullptr;
}

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	this->width = width;
	this->height = height;

	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	saveSystem.Load();
	for (int i = 0; i < saveSystem.entitiesCount; ++i)
	{
		entities.push_back(std::make_shared<Entity>());
	}
	for (int i = 0; i < saveSystem.lightsCount; ++i)
	{
		lights.push_back(std::make_shared<Light>());
	}
	for (int i = 0; i < saveSystem.pointLightsCount; ++i)
	{
		pointlights.push_back(std::make_shared<Light>());
	}

	physicsHandler.Initialize(camera);
	saveSystem.LoadEntityData(entities);
	saveSystem.LoadLightData(lights, pointlights);
	saveSystem.LoadCollisionObjectData(collisionObjects);

	for (int i = 0; i < entities.size(); ++i)
	{
		if (!entities[i]->isDeleted)
		{
			if (entities[i]->entityName == " ")
			{
				entities[i]->entityName = "Entity" + std::to_string(i);
			}
		}
		if (entities[i]->model.isAttached)
		{
			for (int j = 0; j < entities.size(); ++j)
			{
				if (entities[i]->parentName == entities[j]->entityName)
				{
					entities[i]->parent = entities[j].get();
				}
			}
			
		}
		if (entities[i]->physicsComponent.isCharacter && entities[i]->isPlayer)
		{
			player = entities[i];
		}
	}
	if (!renderer.Initialize(this->render_window.GetHWND(), camera, width, height, entities, lights, pointlights))
		return false;

	for (int i = 0; i < collisionObjects.size(); ++i)
	{
		collisionObjects[i].Initialize(renderer.gfx11.device.Get());
	}

	renderer.InitScene(entities, lights,pointlights, camera,sky);
	physicsHandler.CreatePhysicsComponents(entities, collisionObjects);
	
	grid.InitializeBoundsVolume(renderer.gfx11.device.Get());



	tpsPlayerController.Intitialize(renderer.gfx11.device.Get());
	backGroundSound.Initialize("./Data/Sounds/through space.ogg",true, renderer.gfx11.device.Get());
	backGroundSound.cube.pos = DirectX::XMFLOAT3(6.39f, 2.0f, 4.34f);
	sounds.push_back(&backGroundSound);

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update(int width, int height)
{
	ImGuiIO& io = ImGui::GetIO();

	//float dt = timer.GetMilliseconds();

	float dt = 1000.0f / io.Framerate;
	float fps = io.Framerate;
	timer.Restart();
	
	if (keyboard.KeyIsPressed(VK_ESCAPE))
	{
		this->render_window.~RenderWindow();
	}

	float cameraSpeed = 0.001f;



	if (!camera.PossessCharacter)
	{
		if (!renderer.environmentProbe.recalculate)
		{
			renderer.bGuiEnabled = true;
			while (!mouse.EventBufferIsEmpty())
			{
				MouseEvent me = mouse.ReadEvent();

				if (mouse.IsMiddleDown())
				{
					if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
					{
						camera.AdjustRotation(static_cast<float>(me.GetPosY()) * 0.004f, static_cast<float>(me.GetPosX()) * 0.004f, 0.0f, true);
					}
				}
			}

			if (keyboard.KeyIsPressed(VK_SHIFT))
			{
				cameraSpeed = 0.01;
			}
			if (keyboard.KeyIsPressed('W'))
			{
				camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed * dt);
			}
			if (keyboard.KeyIsPressed('S'))
			{
				camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed * dt);
			}

			if (keyboard.KeyIsPressed('A'))
			{
				camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed * dt);

			}
			if (keyboard.KeyIsPressed('D'))
			{
				camera.AdjustPosition(camera.GetRightVector() * cameraSpeed * dt);
			}

			if (keyboard.KeyIsPressed(VK_SPACE))
			{
				camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
			}
			if (keyboard.KeyIsPressed('Q'))
			{
				camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
			}
		}
		
		if (renderer.environmentProbe.recalculate)
		{
			camTempPos = camera.pos;
			camTempRot = camera.rot;
			camera.SetPosition(1.0f, 1.0f, 1.0f);
			camera.SetRotation(0, 0, 0);
			bProbesReady = true;
		}
		

	
		ClipCursor(NULL);
		while (ShowCursor(TRUE) < 0);

	}
	else
	{
		renderer.bGuiEnabled = false;
		RECT rect;
		GetClientRect(this->render_window.GetHWND(), &rect);
		MapWindowPoints(this->render_window.GetHWND(), nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
		
		while (ShowCursor(FALSE) >= 0);
	}

	if (keyboard.KeyIsPressed(VK_F7))
	{
		renderer.bEnableSimulation = true;
		renderer.runPhysics = true;
	}
	if (keyboard.KeyIsPressed(VK_F6))
	{
		renderer.bEnableSimulation = false;
		renderer.runPhysics = false;
	}

	if (keyboard.KeyIsPressed(VK_F5) || renderer.save)
	{
		saveSystem.Save(entities, lights, pointlights, collisionObjects);
		renderer.save = false;
	}

	if (bCanCopy)
	{
		if (keyboard.KeyIsPressed(VK_CONTROL) && keyboard.KeyIsPressed('C'))
		{
			copyEntity = true;
			bCanCopy = false;
		}
	}
	if (!keyboard.KeyIsPressed(VK_CONTROL) && !keyboard.KeyIsPressed('C'))
	{
		bCanCopy = true;
	}
	
	if (bCanPaste)
	{
		if (keyboard.KeyIsPressed(VK_CONTROL) && keyboard.KeyIsPressed('V'))
		{
			pasteEntity = true;
			bCanPaste = false;
		}
	}
	if (!keyboard.KeyIsPressed(VK_CONTROL) && !keyboard.KeyIsPressed('V'))
	{
		bCanPaste = true;
	}

	RenderFrame(dt, fps);


	if (keyboard.KeyIsPressed(VK_RETURN))
	{
		renderer.listbox_item_current = -1;
		for (int i = 0; i < entities.size(); ++i)
		{
			entities[i]->isSelected = false;
			if (entities[i]->physicsComponent.aActor)
			{
				physx::PxShape* _shape = nullptr;
				entities[i]->physicsComponent.aActor->getShapes(&_shape, entities[i]->physicsComponent.aActor->getNbShapes());
				_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);


			}
			else if (entities[i]->physicsComponent.aStaticActor)
			{
				physx::PxShape* _shape = nullptr;
				entities[i]->physicsComponent.aStaticActor->getShapes(&_shape, entities[i]->physicsComponent.aStaticActor->getNbShapes());
				_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
			}
		}

	}

	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i]->bFlagForDeletion)
		{
			entities[i]->Clear(*physicsHandler.aScene);
			if (i < entities.size() - 1)
			{
				std::swap(entities.at(i), entities.back());
			}
			entities.pop_back();
			entities[i]->bFlagForDeletion = false;
		}
	}
	for (int i = 0; i < lights.size(); ++i)
	{
		if (lights[i]->bFlagForDeletion)
		{
			lights[i]->Clear();
			if (i < lights.size() - 1)
			{
				std::swap(lights[i], lights.back());
			}
			lights.pop_back();
			lights[i]->bFlagForDeletion = false;
		}
	}

	for (int i = 0; i < pointlights.size(); ++i)
	{
		if (pointlights[i]->bFlagForDeletion)
		{
			pointlights[i]->Clear();
			if (i < pointlights.size() - 1)
			{
				std::swap(pointlights[i], pointlights.back());
			}
			pointlights.pop_back();
			pointlights[i]->bFlagForDeletion = false;
		}
	}

}

void Engine::RenderFrame(float& dt,float& fps)
{
	float pointX = (float)mouse.GetPosX();
	float pointY = (float)mouse.GetPosY();

	if (renderer.bAddEntity)
	{
		entities.push_back(std::make_shared<Entity>());
		entities[entities.size() - 1]->filePath = renderer.inName;
		int nameIndex = 0;
		for (int i = 0; i < entities.size(); ++i)
		{
			if (entities[i]->entityName == "Entity" + std::to_string(entities.size() + nameIndex))
			{
				nameIndex++;
			}
		}
		entities[entities.size() - 1]->entityName = "Entity" + std::to_string(entities.size() + nameIndex);
		entities[entities.size() - 1]->model.isTextured = renderer.hasTexture;
		entities[entities.size() - 1]->model.isDDS = renderer.isDDS;
		entities[entities.size() - 1]->model.loadAsync = true;

		AddEntity(renderer.inName, renderer.isAnimated, renderer.bConvertCordinates);
	
		renderer.bAddEntity = false;
	}
	if (copyEntity || pasteEntity)
	{
		CopyPasteEntity();
		if (copyEntity)
			copyEntity = false;
		if (pasteEntity)
			pasteEntity = false;
	}

	if (renderer.copyLight)
		CopyPasteLight();
	if (renderer.copyPointLight)
		CopyPastePointLight();

	if (renderer.bAddLight)
	{
		AddLight();
		renderer.bAddLight = false;
	}

	if (renderer.bAddPointLight)
	{
		AddPointLight();
		renderer.bAddPointLight = false;
	}

	if (renderer.bAddCollisionObject)
	{
		AddCollisionObject();
		renderer.bAddCollisionObject = false;
	}

	
	ObjectsHandler(dt);
	AIHandler(dt);
	PlayerLogic(dt);
	GameSounds();

	renderer.Render(camera, entities, physicsHandler, lights, pointlights, collisionObjects, grid, navMeshes, sounds,sky);

	if (bProbesReady)
	{
		bProbesReady = false;
		camera.SetPosition(camTempPos.x, camTempPos.y, camTempPos.z);
		camera.SetRotation(camTempRot.x, camTempRot.y, camTempRot.z);
	}

	sky.pos.x = camera.pos.x;
	sky.pos.y = camera.pos.y;
	sky.pos.z = camera.pos.z;

	if (physicsHandler.advance(dt, fps, camera))
	{
		if (!physicsHandler.isMouseHover && !renderer.bEnableSimulation)
		{
			physicsHandler.MouseRayCast(entities, camera, mouse, keyboard, this->width, this->height, renderer.listbox_item_current);
		}
		if (renderer.bEnableSimulation)
		{
			//async_fireRayCast = std::async(std::launch::async, &Engine::Async_FireRayCast, this);
			Async_FireRayCast();
		}	
	}
}

void Engine::AddEntity(std::string& _inName,bool& isAnimated, bool& bConvertCordinates)
{
	entities[entities.size() - 1]->model.isDDS = renderer.isDDS;
	entities[entities.size() - 1]->model.bConvertCordinates = bConvertCordinates;
	entities[entities.size() - 1]->Intitialize(_inName, renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader, isAnimated);
}

void Engine::AddPhysicsComp(std::shared_ptr<Entity>& entity)
{
	if (entity->physicsComponent.aActor)
	{
		physicsHandler.aScene->removeActor(*entity->physicsComponent.aActor);
	}
	else if (entity->physicsComponent.aStaticActor)
	{
		physicsHandler.aScene->removeActor(*entity->physicsComponent.aStaticActor);
	}

	entity->CreatePhysicsComponent(*physicsHandler.mPhysics, *physicsHandler.aScene);
	entity->physicsComponent.bCreatePhysicsComp = false;

}

void Engine::AddLight()
{
	//lights.push_back(Light());
	//lights[lights.size() - 1].lightType = 1.0f;

	int width, height;

	if (lights[lights.size() - 1]->lightType == 2)
	{
		width = 2048 * 2;
		height = 2048 * 2;
	}
	else
	{
		width = 1024;
		height = 1024;
	}
	width = 2048 * 4;
	height = 2048 * 4;
	lights[lights.size() - 1]->Initialize(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader);
	lights[lights.size() - 1]->m_shadowMap.InitializeShadow(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), width, height, DXGI_FORMAT_R16_FLOAT);
	lights[lights.size() - 1]->SetupCamera(renderer.gfx11.windowWidth, renderer.gfx11.windowHeight);
}
void Engine::AddPointLight()
{
	pointlights.push_back(std::make_shared<Light>());
	pointlights[pointlights.size() - 1]->Initialize(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader);
}

void Engine::AddCollisionObject()
{
	collisionObjects.push_back(CollisionObject());
	collisionObjects[collisionObjects.size() - 1].Initialize(renderer.gfx11.device.Get());
	collisionObjects[collisionObjects.size() - 1].CreatePhysicsComponent(*physicsHandler.mPhysics, *physicsHandler.aScene);
}

void Engine::ObjectsHandler(float& dt)
{
	for (int i = 0; i < entities.size(); ++i)
	{
		entities[i]->model.LoadTextures(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), globalTextureStorage);

		if (entities[i]->isDeleted)
			continue;
		if (entities[i]->physicsComponent.isCharacter && entities[i]->isPlayer)
		{
			player = entities[i];
		}
		if (entities[i]->physicsComponent.bCreatePhysicsComp)
		{
			AddPhysicsComp(entities[i]);
		}
		if (entities[i]->physicsComponent.isCharacter)
		{
			if (entities[i]->bCreateController)
			{
				//entities[i]->entityName = "Entity" + std::to_string(i);
				entities[i]->physicsComponent.CreateController(*physicsHandler.mPhysics, *physicsHandler.aScene, physx::PxVec3(entities[i]->pos.x, entities[i]->pos.y, entities[i]->pos.z), entities[i]->entityName);
				entities[i]->bCreateController = false;
			}
		}
		

		if (!renderer.bEnableSimulation)
		{
			if (entities[i]->physicsComponent.aActor)
			{
				if (!renderer.runPhysics)
				{
					entities[i]->physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
				}

				if (entities[i]->physicsComponent.aActor->getNbShapes() > 0)
				{
					entities[i]->physicsComponent.aActor->getShapes(&entities[i]->physicsComponent.aShape, entities[i]->physicsComponent.aActor->getNbShapes());

					if (entities[i]->physicsComponent.aShape)
						entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);

					if (entities[i]->physicsComponent.aActor)
						entities[i]->physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
				}
			}
			else if (entities[i]->physicsComponent.aStaticActor)
			{
				if (!renderer.runPhysics)
				{
					entities[i]->physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
				}

				if (entities[i]->physicsComponent.aStaticActor->getNbShapes() > 0)
				{
					entities[i]->physicsComponent.aStaticActor->getShapes(&entities[i]->physicsComponent.aShape, entities[i]->physicsComponent.aStaticActor->getNbShapes());

					if (entities[i]->physicsComponent.aShape)
						entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);

					if (entities[i]->physicsComponent.aStaticActor)
						entities[i]->physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
				}
				
			}
		}
		else
		{
			if (entities[i]->physicsComponent.aActor)
			{
				if (renderer.runPhysics)
				{
					entities[i]->physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
				}

				if (entities[i]->physicsComponent.aActor->getNbShapes() > 0)
				{
					entities[i]->physicsComponent.aActor->getShapes(&entities[i]->physicsComponent.aShape, entities[i]->physicsComponent.aActor->getNbShapes());

					if (entities[i]->physicsComponent.aShape)
						entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

					entities[i]->physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
				}
				
			}
			else if (entities[i]->physicsComponent.aStaticActor)
			{
				if (renderer.runPhysics)
				{
					entities[i]->physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
				}
				if (entities[i]->physicsComponent.aStaticActor->getNbShapes() > 0)
				{
					entities[i]->physicsComponent.aStaticActor->getShapes(&entities[i]->physicsComponent.aShape, entities[i]->physicsComponent.aStaticActor->getNbShapes());

					if (entities[i]->physicsComponent.aShape)
						entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

					entities[i]->physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
				}
			}
		}

		entities[i]->physicsComponent.UpdatePhysics(*physicsHandler.mPhysics, *physicsHandler.aScene);
		entities[i]->Update(renderer.bEnableSimulation);

		if (entities[i]->model.isAttached)
		{
			if (entities[i]->parent)
			{
				if (!entities[i]->parentName.empty() && (entities[i]->parent->entityName == entities[i]->parentName))
					entities[i]->SetupAttachment(entities[i]->parent);
				else
				{
					for (int j = 0; j < entities.size(); ++j)
					{
						if (entities[j]->entityName == entities[i]->parentName)
							entities[i]->SetupAttachment(entities[j].get());
					}
				}
			}
		}

		if (entities[i]->physicsComponent.aStaticActor)
		{
			if (renderer.bEnableSimulation)
			{
				if (entities[i]->physicsComponent.physicsShapeEnum == PhysicsShapeEnum::NONE && !entities[i]->physicsComponent.isCharacter)
				{
					entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
				}
			}
			else
			{
				if (entities[i]->physicsComponent.physicsShapeEnum == PhysicsShapeEnum::NONE && !entities[i]->physicsComponent.isCharacter)
				{
					entities[i]->physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				}
			}
		}
		
	}
	//for (int i = 0; i < collisionObjects.size(); ++i)
	//{
	//
	//	collisionObjects[i].physicsComponent.UpdatePhysics(*physicsHandler.mPhysics, *physicsHandler.aScene, physicsHandler.mCooking);
	//	if (renderer.bEnableSimulation)
	//		collisionObjects[i].bRender = false;
	//	else
	//		collisionObjects[i].bRender = true;
	//	collisionObjects[i].physicsComponent.aStaticActor->setName(collisionObjects[i].entityName.c_str());
	//}
}

void Engine::AIHandler(float& dt)
{

	if (entities.empty())
		return;

	grid.SetupGridBounds();


	if (renderer.bCreateGrid)
	{
		if (async_rayCastNavMesh._Is_ready())
		{
			//grid.bGridInitStage = false;
			grid.bRayCastStage = false;
			grid.bCreatePathStage = false;

			renderer.bCreateGrid = false;
			grid.bInitGrid = true;
			
		}
		grid.bCreateOnInit = true;
	}

	if (grid.bCreateOnInit)
	{
		if (grid.bInitGrid)
		{
			AIEntities.clear();
			for (int i = 0; i < entities.size(); ++i)
			{
				if (entities[i]->isAI)
				{
					AIEntities.push_back(entities[i]);
				}
			}

			grid.nodes.clear();
			navMeshes.clear();
			async_navMesh.clear();
			navMeshes.resize(AIEntities.size());
			async_navMesh.resize(AIEntities.size());
			grid.bInitGrid = false;

			grid.Initialize(renderer.gfx11.device, renderer.gfx11.deviceContext, DirectX::XMMatrixIdentity(), renderer.gfx11.cb_vs_vertexshader);
		}

		if (!grid.bRayCastStage)
		{
			async_rayCastNavMesh = std::async(std::launch::async, &GridClass::InitGrid, std::ref(grid));
			grid.bRayCastStage = true;
		}

		if (async_rayCastNavMesh._Is_ready() && grid.bRayCastStage)
		{
			if (!grid.bCreatePathStage)
			{
				physicsHandler.NavMeshRayCast(grid, entities, collisionObjects);

				OutputDebugStringA("\nNavMesh Init!!!!!!!\n");
				for (int i = 0; i < navMeshes.size(); ++i)
				{
					async_navMesh[i] = std::async(std::launch::async, &GridClass::CreatePathGrid, &grid, std::ref(navMeshes[i].validNodes));
				}
				grid.bCreatePathStage = true;
			}
		}
	}
		
		

	if (renderer.bEnableSimulation)
	{
		for (int i = 0; i < AIEntities.size(); ++i)
		{
			if (AIEntities[i]->physicsComponent.aActor)
			{
				float gravity = physicsHandler.aScene->getGravity().y;
						
				physicsHandler.FallCheck(AIEntities[i].get());
				physicsHandler.LineOfSightToPlayer(AIEntities[i].get(), player.get());
						
				if (async_navMesh[i]._Is_ready())
				{
					if (!navMeshes[i].CalculatePath(dt, AIEntities[i].get(), player.get(), enemyController, grid, gravity))
						OutputDebugStringA("Error in A* calculations!\n");
				}
					
			}
		}
	}
}

void Engine::CopyPasteEntity()
{
	if (copyEntity)
	{
		for (int i = 0; i < entities.size(); ++i)
		{
			if (renderer.listbox_item_current == i || entities[i]->isSelected)
			{
				entityToCopy = i;
			}
		}
	}
	if (pasteEntity)
	{

		entities.push_back(std::make_shared<Entity>());
		entities[entities.size() - 1]->CopyData(entities[entityToCopy]);

		int nameIndex = 0;
		for (int i = 0; i < entities.size(); ++i)
		{
			if (entities[i]->entityName == "Entity" + std::to_string(entities.size()+nameIndex))
			{
				nameIndex++;
			}
		}
		entities[entities.size() - 1]->entityName = "Entity" + std::to_string(entities.size()+ nameIndex);

		if (entities[entities.size() - 1]->physicsComponent.physicsShapeEnum == PhysicsShapeEnum::CONVEXMESH ||
			entities[entities.size() - 1]->physicsComponent.physicsShapeEnum == PhysicsShapeEnum::TRIANGLEMESH)
		{
			entities[entities.size() - 1]->model.loadAsync = false;
		}
		else
			entities[entities.size() - 1]->model.loadAsync = true;

		AddEntity(entities[entities.size() - 1]->filePath, entities[entities.size() - 1]->isAnimated, entities[entities.size() - 1]->model.bConvertCordinates);
	
		if (entities[entities.size() - 1]->physicsComponent.isCharacter)
			entities[entities.size() - 1]->physicsComponent.CreateController(*physicsHandler.mPhysics, *physicsHandler.aScene, physx::PxVec3(entities[entities.size() - 1]->pos.x, entities[entities.size() - 1]->pos.y, entities[entities.size() - 1]->pos.z), entities[entities.size() - 1]->entityName);
		else
			entities[entities.size() - 1]->CreatePhysicsComponent(*physicsHandler.mPhysics, *physicsHandler.aScene);

		if (entities[entities.size() - 1]->physicsComponent.aActor)
		{
			entities[entities.size() - 1]->physicsComponent.aActor->setGlobalPose(entities[entities.size() - 1]->physicsComponent.trans);
		}
		else if (entities[entities.size() - 1]->physicsComponent.aStaticActor)
		{
			entities[entities.size() - 1]->physicsComponent.aStaticActor->setGlobalPose(entities[entities.size() - 1]->physicsComponent.trans);
		}
		//OutputDebugStringA("Data Pasted!!\n");
	}

	
}

void Engine::CopyPasteLight()
{
	lights.push_back(std::make_shared<Light>());
	//AddLight();
	lights[lights.size() - 1]->CopyData(lights[renderer.selectedLight]);
	AddLight();
	renderer.copyLight = false;
}


void Engine::CopyPastePointLight()
{
	AddPointLight();

	pointlights[pointlights.size() - 1]->CopyData(pointlights[renderer.selectedPointLight]);
	renderer.copyPointLight = false;

}

void Engine::PlayerLogic(float& dt)
{
	if (player)
	{
		if (renderer.switchCameraMode == 0)
		{
			player->bRender = true;
			tpsPlayerController.MouseMovement(dt, *player, keyboard, mouse, camera);
			tpsPlayerController.Movement(dt, physicsHandler.aScene->getGravity().y, *player, keyboard, mouse, camera);
			tpsPlayerController.Actions(keyboard, mouse, camera);
		}
		else
		{
			player->bRender = false;
			fpsPlayerController.MouseMovement(dt, *player, keyboard, mouse, camera);
			fpsPlayerController.Movement(dt, physicsHandler.aScene->getGravity().y, player.get(), keyboard, mouse, camera);
		}
		async_playerFallCheck = std::async(std::launch::async, &PhysicsHandler::FallCheck, &physicsHandler, player.get());
	}
}

void Engine::GameSounds()
{
	tpsPlayerController.UpdateSounds(camera, player.get());
	backGroundSound.UpdatePos(camera.GetPositionFloat3(), camera.GetForwardVector(), camera.upDir);
	backGroundSound.Async_Update();

	bool fResult;
	backGroundSound.channel->isPlaying(&fResult);
	if (!fResult)
	{
		backGroundSound.Async_Play();
	}
}

void Engine::Async_FireRayCast()
{
	physicsHandler.CrosshairRayTrace(tpsPlayerController, player.get(), entities, renderer.crosshair, camera);
	physicsHandler.PlayerFireRayTrace(tpsPlayerController, player.get(), entities, camera);
}

void Engine::Shutdown()
{
	for (int i = 0; i < entities.size(); ++i)
	{
		entities[i]->Clear(*physicsHandler.aScene);
	}
	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i]->Clear();
	}
	for (int i = 0; i < pointlights.size(); ++i)
	{
		pointlights[i]->Clear();
	}
}
