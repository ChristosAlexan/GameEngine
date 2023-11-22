#include "Engine.h"

using namespace DirectX;


Engine::Engine()
{
	player = nullptr;
}

template<class T>
inline Engine::Engine(T& lhs, T& rhs)
{
	std::swap(lhs, rhs);
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
		entities.push_back(Entity());
	}


	physicsHandler.Initialize(camera);
	saveSystem.LoadEntityData(entities);
	saveSystem.LoadLightData(lights, pointlights);
	saveSystem.LoadCollisionObjectData(collisionObjects);

	for (int i = 0; i < entities.size(); ++i)
	{
		if (!entities[i].isDeleted)
		{
			if (entities[i].entityName == " ")
			{
				entities[i].entityName = "Entity" + std::to_string(i);
			}
		}
		if (entities[i].model.isAttached)
		{
			for (int j = 0; j < entities.size(); ++j)
			{
				if (entities[i].parentName == entities[j].entityName)
				{
					entities[i].parent = &entities[j];
				}
			}
			
		}
		if (entities[i].physicsComponent.isCharacter && entities[i].isPlayer)
		{
			player = &entities[i];
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
		renderer.runPhysics = true;
	}
	if (keyboard.KeyIsPressed(VK_F6))
	{
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
			entities[i].isSelected = false;
			if (entities[i].physicsComponent.aActor)
			{
				physx::PxShape* _shape = nullptr;
				entities[i].physicsComponent.aActor->getShapes(&_shape, entities[i].physicsComponent.aActor->getNbShapes());
				_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);


			}
			else if (entities[i].physicsComponent.aStaticActor)
			{
				physx::PxShape* _shape = nullptr;
				entities[i].physicsComponent.aStaticActor->getShapes(&_shape, entities[i].physicsComponent.aStaticActor->getNbShapes());
				_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
			}
		}

	}

	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i].bFlagForDeletion)
		{
			entities[i].Clear(*physicsHandler.aScene);
			if (i < entities.size() - 1)
			{
				std::swap(entities[i], entities.back());
			}
			entities.pop_back();
			entities[i].bFlagForDeletion = false;
		}
	}
	for (int i = 0; i < lights.size(); ++i)
	{
		if (lights[i].bFlagForDeletion)
		{
			lights[i].Clear();
			if (i < lights.size() - 1)
			{
				std::swap(lights[i], lights.back());
			}
			lights.pop_back();
			lights[i].bFlagForDeletion = false;
		}
	}

	for (int i = 0; i < pointlights.size(); ++i)
	{
		if (pointlights[i].bFlagForDeletion)
		{
			pointlights[i].Clear();
			if (i < pointlights.size() - 1)
			{
				std::swap(pointlights[i], pointlights.back());
			}
			pointlights.pop_back();
			pointlights[i].bFlagForDeletion = false;
		}
	}

}

void Engine::RenderFrame(float& dt,float& fps)
{
	float pointX = (float)mouse.GetPosX();
	float pointY = (float)mouse.GetPosY();

	if (renderer.bAddEntity)
	{
		entities.push_back(Entity());
		entities[entities.size() - 1].filePath = renderer.inName;
		int nameIndex = 0;
		for (int i = 0; i < entities.size(); ++i)
		{
			if (entities[i].entityName == "Entity" + std::to_string(entities.size() + nameIndex))
			{
				nameIndex++;
			}
		}
		entities[entities.size() - 1].entityName = "Entity" + std::to_string(entities.size() + nameIndex);
		entities[entities.size() - 1].model.isTextured = renderer.hasTexture;
		entities[entities.size() - 1].model.isDDS = renderer.isDDS;
		entities[entities.size() - 1].model.loadAsync = true;

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
		if (!physicsHandler.isMouseHover && !renderer.runPhysics)
		{
			physicsHandler.MouseRayCast(entities, camera, mouse, keyboard, this->width, this->height, renderer.listbox_item_current);
		}
		if (renderer.runPhysics)
		{
			//async_fireRayCast = std::async(std::launch::async, &Engine::Async_FireRayCast, this);
			Async_FireRayCast();
		}	
	}
}

void Engine::AddEntity(std::string& _inName,bool& isAnimated, bool& bConvertCordinates)
{
	entities[entities.size() - 1].model.isDDS = renderer.isDDS;
	entities[entities.size() - 1].model.bConvertCordinates = bConvertCordinates;
	entities[entities.size() - 1].Intitialize(_inName, renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader, isAnimated);
}

void Engine::AddPhysicsComp(Entity& entity)
{
	if (entity.physicsComponent.aActor)
	{
		physicsHandler.aScene->removeActor(*entity.physicsComponent.aActor);
	}
	else if (entity.physicsComponent.aStaticActor)
	{
		physicsHandler.aScene->removeActor(*entity.physicsComponent.aStaticActor);
	}

	entity.CreatePhysicsComponent(*physicsHandler.mPhysics, *physicsHandler.aScene);
	entity.physicsComponent.bCreatePhysicsComp = false;

}

void Engine::AddLight()
{
	lights.push_back(Light());
	lights[lights.size() - 1].lightType = 1.0f;


	lights[lights.size() - 1].Initialize(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader);
	lights[lights.size() - 1].m_shadowMap.InitializeShadow(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), 1024, 1024, DXGI_FORMAT_R16_FLOAT);
	lights[lights.size() - 1].SetupCamera(renderer.gfx11.windowWidth, renderer.gfx11.windowHeight);
}
void Engine::AddPointLight()
{
	pointlights.push_back(Light());
	pointlights[pointlights.size() - 1].Initialize(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), renderer.gfx11.cb_vs_vertexshader);
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
		entities[i].model.LoadTextures(renderer.gfx11.device.Get(), renderer.gfx11.deviceContext.Get(), globalTextureStorage);

		if (entities[i].isDeleted)
			continue;
		if (entities[i].physicsComponent.isCharacter && entities[i].isPlayer)
		{
			player = &entities[i];
		}
		if (entities[i].physicsComponent.bCreatePhysicsComp)
		{
			AddPhysicsComp(entities[i]);
		}
		if (entities[i].physicsComponent.isCharacter)
		{
			if (entities[i].bCreateController)
			{
				//entities[i].entityName = "Entity" + std::to_string(i);
				entities[i].physicsComponent.CreateController(*physicsHandler.mPhysics, *physicsHandler.aScene, physx::PxVec3(entities[i].pos.x, entities[i].pos.y, entities[i].pos.z), entities[i].entityName);
				entities[i].bCreateController = false;
			}
		}
	
		if (!renderer.runPhysics)
		{
			if (entities[i].physicsComponent.aActor)
			{
				entities[i].physicsComponent.aActor->getShapes(&entities[i].physicsComponent.aShape, entities[i].physicsComponent.aActor->getNbShapes());

				if(entities[i].physicsComponent.aShape)
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);

				entities[i].physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
			}
			else if (entities[i].physicsComponent.aStaticActor)
			{
				entities[i].physicsComponent.aStaticActor->getShapes(&entities[i].physicsComponent.aShape, entities[i].physicsComponent.aStaticActor->getNbShapes());

				if(entities[i].physicsComponent.aShape)
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);

				entities[i].physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
			}
		}
		else
		{
			if (entities[i].physicsComponent.aActor)
			{
				entities[i].physicsComponent.aActor->getShapes(&entities[i].physicsComponent.aShape, entities[i].physicsComponent.aActor->getNbShapes());

				if(entities[i].physicsComponent.aShape)
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

				entities[i].physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
			}
			else if (entities[i].physicsComponent.aStaticActor)
			{
				entities[i].physicsComponent.aStaticActor->getShapes(&entities[i].physicsComponent.aShape, entities[i].physicsComponent.aStaticActor->getNbShapes());

				if(entities[i].physicsComponent.aShape)
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

				entities[i].physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
			}
		}
		entities[i].physicsComponent.UpdatePhysics(*physicsHandler.mPhysics, *physicsHandler.aScene);
		entities[i].Update(renderer.runPhysics);

		if (entities[i].model.isAttached)
		{
			if (entities[i].parent)
			{
				if (!entities[i].parentName.empty() && (entities[i].parent->entityName == entities[i].parentName))
					entities[i].SetupAttachment(entities[i].parent);
				else
				{
					for (int j = 0; j < entities.size(); ++j)
					{
						if (entities[j].entityName == entities[i].parentName)
							entities[i].SetupAttachment(&entities[j]);
					}
				}
			}
		}

		if (entities[i].physicsComponent.aStaticActor)
		{
			if (renderer.runPhysics)
			{
				if (entities[i].physicsComponent.physicsShapeEnum == PhysicsShapeEnum::NONE && !entities[i].physicsComponent.isCharacter)
				{
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
				}
			}
			else
			{
				if (entities[i].physicsComponent.physicsShapeEnum == PhysicsShapeEnum::NONE && !entities[i].physicsComponent.isCharacter)
				{
					entities[i].physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				}
			}
		}
		
	}
	//for (int i = 0; i < collisionObjects.size(); ++i)
	//{
	//
	//	collisionObjects[i].physicsComponent.UpdatePhysics(*physicsHandler.mPhysics, *physicsHandler.aScene, physicsHandler.mCooking);
	//	if (renderer.runPhysics)
	//		collisionObjects[i].bRender = false;
	//	else
	//		collisionObjects[i].bRender = true;
	//	collisionObjects[i].physicsComponent.aStaticActor->setName(collisionObjects[i].entityName.c_str());
	//}
}

void Engine::AIHandler(float& dt)
{
	grid.SetupGridBounds();

	if (renderer.bHasFinishedLoading)
	{
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
					if (entities[i].isAI)
					{
						AIEntities.push_back(&entities[i]);
					}
				}
				//async_fallCheck.resize(AIEntities.size());
				//async_lineOfSightToPlayer.resize(AIEntities.size());

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
				async_rayCastNavMesh = std::async(std::launch::async, &Engine::CreateNavMesh, this, std::ref(grid), std::ref(entities), std::ref(collisionObjects));
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
		
		

		if (renderer.runPhysics)
		{
			for (int i = 0; i < AIEntities.size(); ++i)
			{
				if (AIEntities[i])
				{
					if (AIEntities[i]->physicsComponent.aActor)
					{
						float gravity = physicsHandler.aScene->getGravity().y;
						
						physicsHandler.FallCheck(AIEntities[i]);
						physicsHandler.LineOfSightToPlayer(AIEntities[i], player);
					
						if(async_rayCastNavMesh._Is_ready() && grid.bCreatePathStage)
							navMeshes[i].CalculatePath(dt, AIEntities[i], player, enemyController, grid, gravity);
					}
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
			if (renderer.listbox_item_current == i || entities[i].isSelected)
			{
				if (entities[i].physicsComponent.aActor)
				{
					copiedEntityData.trans = entities[i].physicsComponent.aActor->getGlobalPose();
				}
				else if (entities[i].physicsComponent.aStaticActor)
				{
					copiedEntityData.trans = entities[i].physicsComponent.aStaticActor->getGlobalPose();
				}
				copiedEntityData.isTextured = entities[i].model.isTextured;
				copiedEntityData.AnimFilePaths = entities[i].model.animFiles;
				copiedEntityData.bRender = entities[i].bRender;
				copiedEntityData.FilePath = entities[i]._filePath;
				copiedEntityData.isAi = entities[i].isAI;
				copiedEntityData.isAnimated = entities[i].isAnimated;
				copiedEntityData.isCharacter = entities[i].physicsComponent.isCharacter;
				copiedEntityData.isfrustumEnabled = entities[i].isfrustumEnabled;
				copiedEntityData.isObstacle = entities[i].isObstacle;
				copiedEntityData.isPlayer = entities[i].isPlayer;
				copiedEntityData.isTransparent = entities[i].model.isTransparent;
				copiedEntityData.isWalkable = entities[i].isWalkable;
				copiedEntityData.mass = entities[i].physicsComponent.mass;
				copiedEntityData.modelPos = entities[i].modelPos;

				if (copiedEntityData.physicsShapeEnum != PhysicsShapeEnum::CONVEXMESH)
				{
					copiedEntityData.physicsShapeEnum = entities[i].physicsComponent.physicsShapeEnum;
					copiedEntityData.selectedShape = entities[i].physicsComponent.selectedShape;
				}
				else
				{
					copiedEntityData.physicsShapeEnum = PhysicsShapeEnum::NONE;
					copiedEntityData.selectedShape = -1;
				}
				copiedEntityData.physics_rot = entities[i].physicsComponent.physics_rot;
				copiedEntityData.physics_scale = entities[i].physicsComponent.physics_scale;
				copiedEntityData.pos = entities[i].pos;
				copiedEntityData.scale = entities[i].scale;
				copiedEntityData.rot = entities[i].rot;
				copiedEntityData.bConvertCordinates = entities[i].model.bConvertCordinates;
				copiedEntityData.frustumScale = entities[i].frustumScale;
				copiedEntityData.isEmissive = entities[i].isEmissive;
				copiedEntityData.emissiveColor = entities[i].emissiveColor;

				//OutputDebugStringA("Data copied!!\n");
			}
		}
	}
	if (pasteEntity)
	{
		entities.push_back(Entity());
		entities[entities.size() - 1].model.isTextured = copiedEntityData.isTextured;
		entities[entities.size() - 1].filePath = copiedEntityData.FilePath;
		entities[entities.size() - 1].model.animFiles = copiedEntityData.AnimFilePaths;
	
		entities[entities.size() - 1].bRender = copiedEntityData.bRender;
		entities[entities.size() - 1].isAI = copiedEntityData.isAi;
		entities[entities.size() - 1].isAnimated = copiedEntityData.isAnimated;
		entities[entities.size() - 1].physicsComponent.isCharacter = copiedEntityData.isCharacter;
		entities[entities.size() - 1].isfrustumEnabled = copiedEntityData.isfrustumEnabled;
		entities[entities.size() - 1].isObstacle = copiedEntityData.isObstacle;
		entities[entities.size() - 1].isPlayer = copiedEntityData.isPlayer;
		entities[entities.size() - 1].model.isTransparent = copiedEntityData.isTransparent;
		entities[entities.size() - 1].isWalkable = copiedEntityData.isWalkable;
		entities[entities.size() - 1].physicsComponent.mass = copiedEntityData.mass;
		entities[entities.size() - 1].modelPos = copiedEntityData.modelPos;
		entities[entities.size() - 1].physicsComponent.physicsShapeEnum = copiedEntityData.physicsShapeEnum;
		entities[entities.size() - 1].physicsComponent.physics_rot = copiedEntityData.physics_rot;
		entities[entities.size() - 1].physicsComponent.physics_scale = copiedEntityData.physics_scale;
		entities[entities.size() - 1].pos = copiedEntityData.pos;
		entities[entities.size() - 1].scale = copiedEntityData.scale;
		entities[entities.size() - 1].rot = copiedEntityData.rot;
		entities[entities.size() - 1].model.bConvertCordinates = copiedEntityData.bConvertCordinates;
		entities[entities.size() - 1].frustumScale = copiedEntityData.frustumScale;
		entities[entities.size() - 1].isEmissive = copiedEntityData.isEmissive;
		entities[entities.size() - 1].emissiveColor = copiedEntityData.emissiveColor;

		int nameIndex = 0;
		for (int i = 0; i < entities.size(); ++i)
		{
			if (entities[i].entityName == "Entity" + std::to_string(entities.size()+nameIndex))
			{
				nameIndex++;
			}
		}
		entities[entities.size() - 1].entityName = "Entity" + std::to_string(entities.size()+ nameIndex);

		if (entities[entities.size() - 1].physicsComponent.physicsShapeEnum == PhysicsShapeEnum::CONVEXMESH ||
			entities[entities.size() - 1].physicsComponent.physicsShapeEnum == PhysicsShapeEnum::TRIANGLEMESH)
		{
			entities[entities.size() - 1].model.loadAsync = false;
		}
		else
			entities[entities.size() - 1].model.loadAsync = true;

		AddEntity(copiedEntityData.FilePath, copiedEntityData.isAnimated, entities[entities.size() - 1].model.bConvertCordinates);
	
		if (entities[entities.size() - 1].physicsComponent.isCharacter)
			entities[entities.size() - 1].physicsComponent.CreateController(*physicsHandler.mPhysics, *physicsHandler.aScene, physx::PxVec3(entities[entities.size() - 1].pos.x, entities[entities.size() - 1].pos.y, entities[entities.size() - 1].pos.z), entities[entities.size() - 1].entityName);
		else
			entities[entities.size() - 1].CreatePhysicsComponent(*physicsHandler.mPhysics, *physicsHandler.aScene);

		if (entities[entities.size() - 1].physicsComponent.aActor)
		{
			entities[entities.size() - 1].physicsComponent.aActor->setGlobalPose(copiedEntityData.trans);
		}
		else if (entities[entities.size() - 1].physicsComponent.aStaticActor)
		{
			entities[entities.size() - 1].physicsComponent.aStaticActor->setGlobalPose(copiedEntityData.trans);
		}
		//OutputDebugStringA("Data Pasted!!\n");
	}

	
}

void Engine::CopyPasteLight()
{
	for (int i = 0; i < lights.size(); ++i)
	{
		if (renderer.selectedLight == i)
		{

			copiedLightData.bShadow = lights[i].bShadow;
			copiedLightData.cutOff = lights[i].cutOff;
			copiedLightData.dimensions = lights[i].dimensions;
			copiedLightData.direction = lights[i].direction;
			copiedLightData.farZ = lights[i].farZ;
			copiedLightData.fov = lights[i].fov;
			copiedLightData.isLightEnabled = lights[i].isLightEnabled;
			copiedLightData.lightColor = lights[i].lightColor;
			copiedLightData.lightType = lights[i].lightType;
			copiedLightData.nearZ = lights[i].nearZ;
			copiedLightData.pos = lights[i].pos;
			copiedLightData.radius = lights[i].radius;
			copiedLightData.scale = lights[i].scale;
			copiedLightData.SpotDir = lights[i].SpotDir;
		}
	}

	AddLight();

	lights[lights.size()-1].cutOff = copiedLightData.cutOff;
	lights[lights.size()-1].dimensions = copiedLightData.dimensions;
	lights[lights.size()-1].direction = copiedLightData.direction;
	lights[lights.size()-1].farZ = copiedLightData.farZ;
	lights[lights.size()-1].fov = copiedLightData.fov;
	lights[lights.size()-1].isLightEnabled = copiedLightData.isLightEnabled;
	lights[lights.size()-1].lightColor = copiedLightData.lightColor;
	lights[lights.size()-1].lightType = copiedLightData.lightType;
	lights[lights.size()-1].nearZ = copiedLightData.nearZ;
	lights[lights.size()-1].pos = copiedLightData.pos;
	lights[lights.size()-1].radius = copiedLightData.radius;
	lights[lights.size()-1].scale = copiedLightData.scale;
	lights[lights.size()-1].SpotDir = copiedLightData.SpotDir;

	renderer.copyLight = false;
	
}


void Engine::CopyPastePointLight()
{
	for (int i = 0; i < pointlights.size(); ++i)
	{
		if (renderer.selectedPointLight == i)
		{
			copiedPointLightData.bShadow = pointlights[i].bShadow;
			copiedPointLightData.cutOff = pointlights[i].cutOff;
			copiedPointLightData.dimensions = pointlights[i].dimensions;
			copiedPointLightData.direction = pointlights[i].direction;
			copiedPointLightData.farZ = pointlights[i].farZ;
			copiedPointLightData.fov = pointlights[i].fov;
			copiedPointLightData.isLightEnabled = pointlights[i].isLightEnabled;
			copiedPointLightData.lightColor = pointlights[i].lightColor;
			copiedPointLightData.lightType = pointlights[i].lightType;
			copiedPointLightData.nearZ = pointlights[i].nearZ;
			copiedPointLightData.pos = pointlights[i].pos;
			copiedPointLightData.radius = pointlights[i].radius;
			copiedPointLightData.scale = pointlights[i].scale;
			copiedPointLightData.SpotDir = pointlights[i].SpotDir;
		}
	}

	AddPointLight();

	pointlights[pointlights.size() - 1].bShadow = copiedPointLightData.bShadow;
	pointlights[pointlights.size() - 1].cutOff = copiedPointLightData.cutOff;
	pointlights[pointlights.size() - 1].dimensions = copiedPointLightData.dimensions;
	pointlights[pointlights.size() - 1].direction = copiedPointLightData.direction;
	pointlights[pointlights.size() - 1].farZ = copiedPointLightData.farZ;
	pointlights[pointlights.size() - 1].fov = copiedPointLightData.fov;
	pointlights[pointlights.size() - 1].isLightEnabled = copiedPointLightData.isLightEnabled;
	pointlights[pointlights.size() - 1].lightColor = copiedPointLightData.lightColor;
	pointlights[pointlights.size() - 1].lightType = copiedPointLightData.lightType;
	pointlights[pointlights.size() - 1].nearZ = copiedPointLightData.nearZ;
	pointlights[pointlights.size() - 1].pos = copiedPointLightData.pos;
	pointlights[pointlights.size() - 1].radius = copiedPointLightData.radius;
	pointlights[pointlights.size() - 1].scale = copiedPointLightData.scale;
	pointlights[pointlights.size() - 1].SpotDir = copiedPointLightData.SpotDir;

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
			fpsPlayerController.Movement(dt, physicsHandler.aScene->getGravity().y, player, keyboard, mouse, camera);
		}
		//physicsHandler.FallCheck(player);

		async_playerFallCheck = std::async(std::launch::async, &PhysicsHandler::FallCheck, &physicsHandler, player);
	}
}

void Engine::GameSounds()
{
	tpsPlayerController.UpdateSounds(camera, player);
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
	physicsHandler.CrosshairRayTrace(tpsPlayerController, player, entities, renderer.crosshair, camera);
	physicsHandler.PlayerFireRayTrace(tpsPlayerController, player, entities, camera);
}

void Engine::CreateNavMesh(GridClass& grid, std::vector<Entity>& entities, std::vector<CollisionObject>& collisionObjects)
{
	grid.InitGrid();
	/*physicsHandler.NavMeshRayCast(grid, entities, collisionObjects);

	OutputDebugStringA("\nNavMesh Init!!!!!!!\n");*/
}

void Engine::Shutdown()
{
	for (int i = 0; i < entities.size(); ++i)
	{
		entities[i].Clear(*physicsHandler.aScene);
	}
	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i].Clear();
	}
	for (int i = 0; i < pointlights.size(); ++i)
	{
		pointlights[i].Clear();
	}
}
