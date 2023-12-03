#include "SaveSystem.h"
#include <stdio.h>
#include <filesystem>

SaveSystem::SaveSystem()
{
}

void SaveSystem::Save(std::vector<std::shared_ptr<Entity>>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, std::vector<CollisionObject>& collisionObject)
{
	std::ofstream outfile;
	outfile = std::ofstream("Files/File/GeneralSettings.txt");

	if (outfile.is_open())
	{
		outfile << "Count= " << entities.size() << "\n";
		outfile << "lightsCount= " << lights.size() << "\n";
		outfile << "pointLightsCount= " << pointLights.size() << "\n";
		outfile << "CollisionCount= " << collisionObject.size() << "\n";
	}

	auto dirIter = std::filesystem::directory_iterator("Files/File/Entities");
	int fileCount = 0;

	for (auto& entry : dirIter)
	{
		if (entry.is_regular_file())
		{
			++fileCount;
		}
	}

	for (int i = entities.size(); i <= fileCount; ++i)
	{
		if (!std::remove(("Files/File/Entities/Entity" + std::to_string(i) + ".txt").c_str()))
		{
			
		}
	}

	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i]->isDeleted)
			continue;
		outfile = std::ofstream("Files/File/Entities/Entity" + std::to_string(i) + ".txt");

		if (outfile.is_open())
		{
			outfile << "mass= " << entities[i]->physicsComponent.mass << "\n";

			if (entities[i]->physicsComponent.aActor || entities[i]->physicsComponent.aStaticActor || entities[i]->physicsComponent.isCharacter)
			{
				outfile << "posX= " << entities[i]->physicsComponent.trans.p.x << "\n";
				outfile << "posY= " << entities[i]->physicsComponent.trans.p.y << "\n";
				outfile << "posZ= " << entities[i]->physicsComponent.trans.p.z << "\n";
			}
			else
			{
				outfile << "posX= " << entities[i]->pos.x << "\n";
				outfile << "posY= " << entities[i]->pos.y << "\n";
				outfile << "posZ= " << entities[i]->pos.z << "\n";
			}
			
			outfile << "rotX= " << entities[i]->rot.x << "\n";
			outfile << "rotY= " << entities[i]->rot.y << "\n";
			outfile << "rotZ= " << entities[i]->rot.z << "\n";

			outfile << "scaleX= " << entities[i]->scale.x << "\n";
			outfile << "scaleY= " << entities[i]->scale.y << "\n";
			outfile << "scaleZ= " << entities[i]->scale.z << "\n";

			outfile << "modelPosX= " << entities[i]->modelPos.x << "\n";
			outfile << "modelPosY= " << entities[i]->modelPos.y << "\n";
			outfile << "modelPosZ= " << entities[i]->modelPos.z << "\n";


			outfile << "physicsRotX= " << entities[i]->physicsComponent.physics_rot.x << "\n";
			outfile << "physicsRotY= " << entities[i]->physicsComponent.physics_rot.y << "\n";
			outfile << "physicsRotZ= " << entities[i]->physicsComponent.physics_rot.z << "\n";
			outfile << "physicsRotW= " << entities[i]->physicsComponent.physics_rot.w << "\n";

			outfile << "physicsScaleX= " << entities[i]->physicsComponent.physics_scale.x << "\n";
			outfile << "physicsScaleY= " << entities[i]->physicsComponent.physics_scale.y << "\n";
			outfile << "physicsScaleZ= " << entities[i]->physicsComponent.physics_scale.z << "\n";


			outfile << "frustumScaleX= " << entities[i]->frustumScale.x << "\n";
			outfile << "frustumScaleY= " << entities[i]->frustumScale.y << "\n";
			outfile << "frustumScaleZ= " << entities[i]->frustumScale.z << "\n";

			outfile << "emissiveColorX= " << entities[i]->emissiveColor.x << "\n";
			outfile << "emissiveColorY= " << entities[i]->emissiveColor.y << "\n";
			outfile << "emissiveColorZ= " << entities[i]->emissiveColor.z << "\n";

			outfile << "isCharacter= " << entities[i]->physicsComponent.isCharacter << "\n";
			outfile << "isPlayer= " << entities[i]->isPlayer << "\n";
			outfile << "isAI= " << entities[i]->isAI << "\n";
			outfile << "isWalkable= " << entities[i]->isWalkable << "\n";
			outfile << "isObstacle= " << entities[i]->isObstacle << "\n";
			outfile << "Render= " << entities[i]->bRender << "\n";
			outfile << "isAnimated= " << entities[i]->isAnimated << "\n";
			outfile << "ConvertCordinates= " << entities[i]->model.bConvertCordinates << "\n";
			outfile << "isAttached= " << entities[i]->model.isAttached << "\n";
			outfile << "Frustum= " << entities[i]->isfrustumEnabled << "\n";
			outfile << "isDeleted= " << entities[i]->isDeleted << "\n";
			outfile << "isEmissive= " << entities[i]->isEmissive << "\n";
			outfile << "isTransparent= " << entities[i]->model.isTransparent << "\n";
			outfile << "isTextured= " << entities[i]->model.isTextured << "\n";
			outfile << "isDDS= " << entities[i]->model.isDDS << "\n";
			outfile << "physicsShape= " << entities[i]->physicsComponent.physicsShapeEnum << "\n";
			outfile << "animFilesCount= " << entities[i]->model.animFiles.size() << "\n";

			outfile << "entityName= " << entities[i]->entityName << "\n";
			if (entities[i]->model.isAttached)
			{
				if(entities[i]->parent && !entities[i]->parent->entityName.empty())
					outfile << "attachedEntityName= " << entities[i]->parent->entityName << "\n";

				if(!entities[i]->attachedBone.empty())
					outfile << "attachedBone= " << entities[i]->attachedBone << "\n";
			}
			std::string _path;

			size_t npos = entities[i]->filePath.find("Data");
			_path = entities[i]->filePath.substr(npos);

			outfile << "filePath= " << ".//" << _path << "\n";

			for (int j = 0; j < entities[i]->model.animFiles.size(); ++j)
			{
				std::string _path;

				size_t npos = entities[i]->model.animFiles[j].find("Data");
				_path = entities[i]->model.animFiles[j].substr(npos);

				outfile << "animFiles" + std::to_string(j) + "= " <<".//" << _path << "\n";
			}

			outfile.close();
		}
	}






	for (int i = 0; i < lights.size(); ++i)
	{
		outfile = std::ofstream("Files/File/Lights/Light" + std::to_string(i) + ".txt");

		if (outfile.is_open())
		{

			outfile << "posX= " << lights[i].pos.x << "\n";
			outfile << "posY= " << lights[i].pos.y << "\n";
			outfile << "posZ= " << lights[i].pos.z << "\n";
			
			outfile << "scaleX= " << lights[i].scale.x << "\n";
			outfile << "scaleY= " << lights[i].scale.y << "\n";
			outfile << "scaleZ= " << lights[i].scale.z << "\n";

			outfile << "colorX= " << lights[i].lightColor.x << "\n";
			outfile << "colorY= " << lights[i].lightColor.y << "\n";
			outfile << "colorZ= " << lights[i].lightColor.z << "\n";
			outfile << "colorW= " << lights[i].lightColor.w << "\n";

			outfile << "emissionColorX= " << lights[i].emissionColor.x << "\n";
			outfile << "emissionColorY= " << lights[i].emissionColor.y << "\n";
			outfile << "emissionColorZ= " << lights[i].emissionColor.z << "\n";

			outfile << "directionX= " << lights[i].direction.x << "\n";
			outfile << "directionY= " << lights[i].direction.y << "\n";
			outfile << "directionZ= " << lights[i].direction.z << "\n";

			outfile << "spotDirX= " << lights[i].SpotDir.x << "\n";
			outfile << "spotDirY= " << lights[i].SpotDir.y << "\n";
			outfile << "spotDirZ= " << lights[i].SpotDir.z << "\n";

			outfile << "shadowsSoftness= " << lights[i].shadowsSoftnessBias.x << "\n";
			outfile << "shadowBias= " << lights[i].shadowsSoftnessBias.y << "\n";

			outfile << "radius= " << lights[i].radius << "\n";
			outfile << "cutOff= " << lights[i].cutOff << "\n";

			outfile << "lightType= " << lights[i].lightType << "\n";
			outfile << "dimensions= " << lights[i].dimensions << "\n";
			outfile << "nearZ= " << lights[i].nearZ << "\n";
			outfile << "farZ= " << lights[i].farZ << "\n";
			outfile << "fov= " << lights[i].fov << "\n";
			outfile.close();
		}
	}

	for (int i = 0; i < pointLights.size(); ++i)
	{
		outfile = std::ofstream("Files/File/PointLights/pointLight" + std::to_string(i) + ".txt");

		if (outfile.is_open())
		{

			outfile << "posX= " << pointLights[i].pos.x << "\n";
			outfile << "posY= " << pointLights[i].pos.y << "\n";
			outfile << "posZ= " << pointLights[i].pos.z << "\n";

			outfile << "scaleX= " << pointLights[i].scale.x << "\n";
			outfile << "scaleY= " << pointLights[i].scale.y << "\n";
			outfile << "scaleZ= " << pointLights[i].scale.z << "\n";

			outfile << "colorX= " << pointLights[i].lightColor.x << "\n";
			outfile << "colorY= " << pointLights[i].lightColor.y << "\n";
			outfile << "colorZ= " << pointLights[i].lightColor.z << "\n";
		

			outfile << "emissionColorX= " << pointLights[i].emissionColor.x << "\n";
			outfile << "emissionColorY= " << pointLights[i].emissionColor.y << "\n";
			outfile << "emissionColorZ= " << pointLights[i].emissionColor.z << "\n";

			outfile << "directionX= " << pointLights[i].direction.x << "\n";
			outfile << "directionY= " << pointLights[i].direction.y << "\n";
			outfile << "directionZ= " << pointLights[i].direction.z << "\n";

			outfile << "spotDirX= " << pointLights[i].SpotDir.x << "\n";
			outfile << "spotDirY= " << pointLights[i].SpotDir.y << "\n";
			outfile << "spotDirZ= " << pointLights[i].SpotDir.z << "\n";

			outfile << "radius= " << pointLights[i].radius << "\n";
			outfile << "cutOff= " << pointLights[i].cutOff << "\n";

			outfile << "lightType= " << pointLights[i].lightType << "\n";
			outfile << "dimensions= " << pointLights[i].dimensions << "\n";
			outfile << "fov= " << pointLights[i].fov << "\n";
			outfile.close();
		}
	}

	for (int i = 0; i < collisionObject.size(); ++i)
	{
		outfile = std::ofstream("Files/File/CollisionObject/collisionObject" + std::to_string(i) + ".txt");

		if (outfile.is_open())
		{
			if (collisionObject[i].physicsComponent.aStaticActor)
			{

				outfile << "posX= " << collisionObject[i].physicsComponent.aStaticActor->getGlobalPose().p.x << "\n";
				outfile << "posY= " << collisionObject[i].physicsComponent.aStaticActor->getGlobalPose().p.y << "\n";
				outfile << "posZ= " << collisionObject[i].physicsComponent.aStaticActor->getGlobalPose().p.z << "\n";
			}

			//outfile << "posX= " << lights[i].pos.x << "\n";
			//outfile << "posY= " << lights[i].pos.y << "\n";
			//outfile << "posZ= " << lights[i].pos.z << "\n";

			outfile << "scaleX= " << collisionObject[i].cube.scale.x << "\n";
			outfile << "scaleY= " << collisionObject[i].cube.scale.y << "\n";
			outfile << "scaleZ= " << collisionObject[i].cube.scale.z << "\n";

			outfile << "physicsScaleX= " << collisionObject[i].physicsComponent.physics_scale.x << "\n";
			outfile << "physicsScaleY= " << collisionObject[i].physicsComponent.physics_scale.y << "\n";
			outfile << "physicsScaleZ= " << collisionObject[i].physicsComponent.physics_scale.z << "\n";

			outfile.close();
		}
	}
}

void SaveSystem::Load()
{
	std::ifstream f;
	std::string path;
	float val = 0.0f;
	f = std::ifstream("Files/File/GeneralSettings.txt");

	if (f.good())
	{
		if (f.is_open())
		{
			while (f >> path >> val)
			{
				if (path == "Count=")
				{
					entitiesCount = (int)val;
				}
				if (path == "lightsCount=")
				{
					lightsCount = (int)val;
				}
				if (path == "pointLightsCount=")
				{
					pointLightsCount = (int)val;
				}
				if (path == "CollisionCount=")
				{
					collisionObjectCount = (int)val;
				}
			}

			f.close();
		}
	}
}

void SaveSystem::LoadEntityData(std::vector<std::shared_ptr<Entity>>& entities)
{
	for (int i = 0; i < entitiesCount; ++i)
	{
		//entities[i]->entityName = "Entity" + std::to_string(i);
		std::ifstream f;
		std::string path="";
		float val = 0;
		std::string str ="";
		f = std::ifstream("Files/File/Entities/Entity"+std::to_string(i)+".txt");

		if (f.good())
		{
			if (f.is_open())
			{
				while (f >> path >> val)
				{
					if (path == "mass=")
					{
						entities[i]->physicsComponent.mass = (float)val;
					}
					if (path == "posX=")
					{
						entities[i]->pos.x = (float)val;
					}
					if (path == "posY=")
					{
						entities[i]->pos.y = (float)val;
					}
					if (path == "posZ=")
					{
						entities[i]->pos.z = (float)val;
					}
					

					//if (i == 2)
					//	OutputDebugStringA(("POS = " + std::to_string(entities[i]->pos.x) + "\n").c_str());
					
					if (path == "rotX=")
					{
						entities[i]->rot.x = (float)val;
					}
					if (path == "rotY=")
					{
						entities[i]->rot.y = (float)val;
					}
					if (path == "rotZ=")
					{
						entities[i]->rot.z = (float)val;
					}

					if (path == "scaleX=")
					{
						entities[i]->scale.x = (float)val;
					}
					if (path == "scaleY=")
					{
						entities[i]->scale.y = (float)val;
					}
					if (path == "scaleZ=")
					{
						entities[i]->scale.z = (float)val;
					}

					if (path == "modelPosX=")
					{
						entities[i]->modelPos.x = (float)val;
					}
					if (path == "modelPosY=")
					{
						entities[i]->modelPos.y = (float)val;
					}
					if (path == "modelPosZ=")
					{
						entities[i]->modelPos.z = (float)val;
					}

					if (path == "physicsRotX=")
					{
						entities[i]->physicsComponent.physics_rot.x = (float)val;
					}
					if (path == "physicsRotY=")
					{
						entities[i]->physicsComponent.physics_rot.y = (float)val;
					}
					if (path == "physicsRotZ=")
					{
						entities[i]->physicsComponent.physics_rot.z = (float)val;
					}
					if (path == "physicsRotW=")
					{
						entities[i]->physicsComponent.physics_rot.w = (float)val;
					}
					if (path == "physicsScaleX=")
					{
						entities[i]->physicsComponent.physics_scale.x = (float)val;
					}
					if (path == "physicsScaleY=")
					{
						entities[i]->physicsComponent.physics_scale.y = (float)val;
					}
					if (path == "physicsScaleZ=")
					{
						entities[i]->physicsComponent.physics_scale.z = (float)val;
					}

					if (path == "frustumScaleX=")
					{
						entities[i]->frustumScale.x = (float)val;
					}
					if (path == "frustumScaleY=")
					{
						entities[i]->frustumScale.y = (float)val;
					}
					if (path == "frustumScaleZ=")
					{
						entities[i]->frustumScale.z = (float)val;
					}

					if (path == "emissiveColorX=")
					{
						entities[i]->emissiveColor.x = (float)val;
					}
					if (path == "emissiveColorY=")
					{
						entities[i]->emissiveColor.y = (float)val;
					}
					if (path == "emissiveColorZ=")
					{
						entities[i]->emissiveColor.z = (float)val;
					}

					if (path == "isCharacter=")
					{
						entities[i]->physicsComponent.isCharacter = (int)val;
					}
					if (path == "isPlayer=")
					{
						entities[i]->isPlayer = (int)val;
					}
					if (path == "isAI=")
					{
						entities[i]->isAI = (int)val;
					}
					if (path == "isWalkable=")
					{
						entities[i]->isWalkable = (int)val;
					}
					if (path == "isObstacle=")
					{
						entities[i]->isObstacle = (int)val;
					}
					if (path == "Render=")
					{
						entities[i]->bRender = (int)val;
					}
					if (path == "isAnimated=")
					{
						entities[i]->isAnimated = (int)val;
					}
					if (path == "ConvertCordinates=")
					{
						entities[i]->model.bConvertCordinates = (int)val;
					}
					if (path == "isAttached=")
					{
						entities[i]->model.isAttached = (int)val;
					}
					if (path == "Frustum=")
					{
						entities[i]->isfrustumEnabled = (int)val;
					}
					if (path == "isDeleted=")
					{
						entities[i]->isDeleted = (int)val;
					}
					if (path == "isEmissive=")
					{
						entities[i]->isEmissive = (int)val;
					}
					if (path == "isTransparent=")
					{
						entities[i]->model.isTransparent = (int)val;
					}
					if (path == "isTextured=")
					{
						entities[i]->model.isTextured = (int)val;
					}
					if (path == "isDDS=")
					{
						entities[i]->model.isDDS = (int)val;
					}
					if (path == "physicsShape=")
					{
						entities[i]->physicsComponent.physicsShapeEnum = (PhysicsShapeEnum)val;
						entities[i]->physicsComponent.selectedShape = (int)val;
					}
					
					if (path == "animFilesCount=")
					{
						entities[i]->model.animFiles.resize((int)val);
					}
				}
				f.close();
				f.clear();
			}
		}

		f = std::ifstream("Files/File/Entities/Entity" + std::to_string(i) + ".txt");

		if (f.good())
		{
			if (f.is_open())
			{
				while (f >> path >> str)
				{
					if (path == "entityName=")
					{
						entities[i]->entityName = "Entity"+std::to_string(i);
					}
					if (path == "attachedEntityName=")
					{
						entities[i]->parentName = str;
					}
					if (path == "attachedBone=")
					{
						entities[i]->attachedBone = str;
					}
					if (path == "filePath=")
					{
						entities[i]->filePath = str;
					}
					for (int j = 0; j < entities[i]->model.animFiles.size(); ++j)
					{
						if (path == "animFiles" + std::to_string(j) + "=")
						{
							//OutputDebugStringA(str.c_str());
							//OutputDebugStringA("\n");
							entities[i]->model.animFiles[j] = str;
						}
					}
				}
				
				f.close();
				f.clear();
			}
		}
	}
	
}

void SaveSystem::LoadLightData(std::vector<Light>& lights, std::vector<Light>& pointLights)
{
	lights.resize(lightsCount);
	for (int i = 0; i < lightsCount; ++i)
	{
		std::ifstream f;
		std::string path = "";
		float val = 0;
		std::string str = "";
		f = std::ifstream("Files/File/Lights/Light" + std::to_string(i) + ".txt");

		if (f.good())
		{
			if (f.is_open())
			{
				while (f >> path >> val)
				{
					if (path == "posX=")
					{
						lights[i].pos.x = (float)val;
					}
					if (path == "posY=")
					{
						lights[i].pos.y = (float)val;
					}
					if (path == "posZ=")
					{
						lights[i].pos.z = (float)val;
					}
					
					if (path == "scaleX=")
					{
						lights[i].scale.x = (float)val;
					}
					if (path == "scaleY=")
					{
						lights[i].scale.y = (float)val;
					}
					if (path == "scaleZ=")
					{
						lights[i].scale.z = (float)val;
					}

					if (path == "colorX=")
					{
						lights[i].lightColor.x = (float)val;
					}
					if (path == "colorY=")
					{
						lights[i].lightColor.y = (float)val;
					}
					if (path == "colorZ=")
					{
						lights[i].lightColor.z = (float)val;
					}
					if (path == "colorW=")
					{
						lights[i].lightColor.w = (float)val;
					}

					if (path == "emissionColorX=")
					{
						lights[i].emissionColor.x = (float)val;
					}
					if (path == "emissionColorY=")
					{
						lights[i].emissionColor.y = (float)val;
					}
					if (path == "emissionColorZ=")
					{
						lights[i].emissionColor.z = (float)val;
					}

					if (path == "directionX=")
					{
						lights[i].direction.x = (float)val;
					}
					if (path == "directionY=")
					{
						lights[i].direction.y = (float)val;
					}
					if (path == "directionZ=")
					{
						lights[i].direction.z = (float)val;
					}

					if (path == "spotDirX=")
					{
						lights[i].SpotDir.x = (float)val;
					}
					if (path == "spotDirY=")
					{
						lights[i].SpotDir.y = (float)val;
					}
					if (path == "spotDirZ=")
					{
						lights[i].SpotDir.z = (float)val;
					}

					if (path == "shadowsSoftness=")
					{
						lights[i].shadowsSoftnessBias.x = (float)val;
					}
					if (path == "shadowBias=")
					{
						lights[i].shadowsSoftnessBias.y = (float)val;
					}

					if (path == "radius=")
					{
						lights[i].radius = (float)val;
					}
					
					if (path == "cutOff=")
					{
						lights[i].cutOff = (float)val;
					}

					if (path == "lightType=")
					{
						lights[i].lightType = (float)val;
					}
					if (path == "dimensions=")
					{
						lights[i].dimensions = (float)val;
					}
					if (path == "nearZ=")
					{
						lights[i].nearZ = (float)val;
					}
					if (path == "farZ=")
					{
						lights[i].farZ = (float)val;
					}
					if (path == "fov=")
					{
						lights[i].fov = (float)val;
					}

				}
				f.close();
				f.clear();
			}
		}
	}








	pointLights.resize(pointLightsCount);
	for (int i = 0; i < pointLightsCount; ++i)
	{
		std::ifstream f;
		std::string path = "";
		float val = 0;
		std::string str = "";
		f = std::ifstream("Files/File/PointLights/pointLight" + std::to_string(i) + ".txt");

		if (f.good())
		{
			if (f.is_open())
			{
				while (f >> path >> val)
				{
					if (path == "posX=")
					{
						pointLights[i].pos.x = (float)val;
					}
					if (path == "posY=")
					{
						pointLights[i].pos.y = (float)val;
					}
					if (path == "posZ=")
					{
						pointLights[i].pos.z = (float)val;
					}

					if (path == "scaleX=")
					{
						pointLights[i].scale.x = (float)val;
					}
					if (path == "scaleY=")
					{
						pointLights[i].scale.y = (float)val;
					}
					if (path == "scaleZ=")
					{
						pointLights[i].scale.z = (float)val;
					}

					if (path == "colorX=")
					{
						pointLights[i].lightColor.x = (float)val;
					}
					if (path == "colorY=")
					{
						pointLights[i].lightColor.y = (float)val;
					}
					if (path == "colorZ=")
					{
						pointLights[i].lightColor.z = (float)val;
					}

					if (path == "emissionColorX=")
					{
						pointLights[i].emissionColor.x = (float)val;
					}
					if (path == "emissionColorY=")
					{
						pointLights[i].emissionColor.y = (float)val;
					}
					if (path == "emissionColorZ=")
					{
						pointLights[i].emissionColor.z = (float)val;
					}

					if (path == "directionX=")
					{
						pointLights[i].direction.x = (float)val;
					}
					if (path == "directionY=")
					{
						pointLights[i].direction.y = (float)val;
					}
					if (path == "directionZ=")
					{
						pointLights[i].direction.z = (float)val;
					}

					if (path == "spotDirX=")
					{
						pointLights[i].SpotDir.x = (float)val;
					}
					if (path == "spotDirY=")
					{
						pointLights[i].SpotDir.y = (float)val;
					}
					if (path == "spotDirZ=")
					{
						pointLights[i].SpotDir.z = (float)val;
					}

					if (path == "radius=")
					{
						pointLights[i].radius = (float)val;
					}

					if (path == "cutOff=")
					{
						pointLights[i].cutOff = (float)val;
					}

					if (path == "lightType=")
					{
						pointLights[i].lightType = (float)val;
					}
					if (path == "dimensions=")
					{
						pointLights[i].dimensions = (float)val;
					}
					if (path == "fov=")
					{
						pointLights[i].fov = (float)val;
					}

				}
				f.close();
				f.clear();
			}
		}
	}
}

void SaveSystem::LoadCollisionObjectData(std::vector<CollisionObject>& collisionObject)
{
	collisionObject.resize(collisionObjectCount);
	for (int i = 0; i < collisionObjectCount; ++i)
	{
		std::ifstream f;
		std::string path = "";
		float val = 0;
		std::string str = "";
		f = std::ifstream("Files/File/CollisionObject/collisionObject" + std::to_string(i) + ".txt");

		if (f.good())
		{
			if (f.is_open())
			{
				while (f >> path >> val)
				{
					if (path == "posX=")
					{
						collisionObject[i].cube.pos.x = (float)val;
					}
					if (path == "posY=")
					{
						collisionObject[i].cube.pos.y = (float)val;
					}
					if (path == "posZ=")
					{
						collisionObject[i].cube.pos.z = (float)val;
					}

					if (path == "scaleX=")
					{
						collisionObject[i].cube.scale.x = (float)val;
					}
					if (path == "scaleY=")
					{
						collisionObject[i].cube.scale.y = (float)val;
					}
					if (path == "scaleZ=")
					{
						collisionObject[i].cube.scale.z = (float)val;
					}

					if (path == "physicsScaleX=")
					{
						collisionObject[i].physicsComponent.physics_scale.x = (float)val;
					}
					if (path == "physicsScaleY=")
					{
						collisionObject[i].physicsComponent.physics_scale.y = (float)val;
					}
					if (path == "physicsScaleZ=")
					{
						collisionObject[i].physicsComponent.physics_scale.z = (float)val;
					}

				}
				f.close();
				f.clear();
			}
		}
	}
}

