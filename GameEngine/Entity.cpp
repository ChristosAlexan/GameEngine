#include "Entity.h"

inline std::string ImGuiIDLabel(const std::string& base, int index) {
	return base + std::to_string(index);
}

Entity::Entity()
{
	pos = DirectX::XMFLOAT3(0, 0, 0);
	scale = DirectX::XMFLOAT3(1, 1, 1);
	rot = DirectX::XMFLOAT3(0, 0, 0);
	modelPos = DirectX::XMFLOAT3(0, 0, 0);
	frustumScale = DirectX::XMFLOAT3(0, 0, 0);
	isAnimated = false;
	bRender = true;
	isDeleted = false;
	bFlagForDeletion = false;
	isEmissive = false;
}

void Entity::CopyData(const std::shared_ptr<Entity>& other)
{
	if (other->physicsComponent.aActor)
	{
		physicsComponent.trans = other->physicsComponent.aActor->getGlobalPose();
	}
	else if (other->physicsComponent.aStaticActor)
	{
		physicsComponent.trans = other->physicsComponent.aStaticActor->getGlobalPose();
	}
	model.isTextured = other->model.isTextured;
	model.animFiles = other->model.animFiles;
	bRender = other->bRender;
	filePath = other->_filePath;
	isAI = other->isAI;
	isAnimated = other->isAnimated;
	physicsComponent.isCharacter = other->physicsComponent.isCharacter;
	isfrustumEnabled = other->isfrustumEnabled;
	isObstacle = other->isObstacle;
	isPlayer = other->isPlayer;
	model.isTransparent = other->model.isTransparent;
	isWalkable = other->isWalkable;
	physicsComponent.mass = other->physicsComponent.mass;
	modelPos = other->modelPos;

	physicsComponent.physicsShapeEnum = other->physicsComponent.physicsShapeEnum;
	physicsComponent.selectedShape = other->physicsComponent.selectedShape;

	physicsComponent.physics_rot = other->physicsComponent.physics_rot;
	physicsComponent.physics_scale = other->physicsComponent.physics_scale;
	pos = other->pos;
	scale = other->scale;
	rot = other->rot;
	model.bConvertCordinates = other->model.bConvertCordinates;
	frustumScale = other->frustumScale;
	isEmissive = other->isEmissive;
	emissiveColor = other->emissiveColor;
}

bool Entity::Intitialize(const std::string filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, bool isAnimated)
{
	frustum.frustumCube.Initialize(device);

	this->_filePath = filePath;
	this->isAnimated = isAnimated;

	if (!model.Initialize(this->filePath, device, deviceContext, cb_vs_vertexshader, this->isAnimated))
	{
		return false;
	}

	return true;
}

void Entity::CreatePhysicsComponent(physx::PxPhysics& physics, physx::PxScene& scene)
{
	if (isDeleted)
		return;

	if (physicsComponent.aActor)
	{
		scene.removeActor(*physicsComponent.aActor);
		scale = DirectX::XMFLOAT3(0, 0, 0);
	}
	else if (physicsComponent.aStaticActor)
	{
		scene.removeActor(*physicsComponent.aStaticActor);
		scale = DirectX::XMFLOAT3(0, 0, 0);
	}

	switch (physicsComponent.physicsShapeEnum)
	{
	case CUBE:
		physicsComponent.CreateCube(physics, scene, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case PLANE:
		physicsComponent.CreatePlane(physics, scene, physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case SPHERE:
		physicsComponent.CreateSphere(physics, scene, physicsComponent.physics_scale.x, physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case CAPSULE:
		physicsComponent.CreateCapsule(physics, scene, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case CONVEXMESH:
		physicsComponent.CreateConvex(physics, scene, model.m_vertices, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case TRIANGLEMESH:
		physicsComponent.CreateTriangleMesh(physics, scene, model.m_vertices, model.m_indices, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	case NONE:
		if(!physicsComponent.isCharacter)
			physicsComponent.CreatePickingShape(physics, scene, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z));
		break;
	}
}

void Entity::UpdatePhysics(bool& runPhysics)
{
	if (isDeleted)
		return;

	if (physicsComponent.aActor)
	{
		physicsComponent.aActor->setName(entityName.c_str());
		physicsComponent.trans = physicsComponent.aActor->getGlobalPose();
	}
	else if (physicsComponent.aStaticActor)
	{
		physicsComponent.aStaticActor->setName(entityName.c_str());
		physicsComponent.trans = physicsComponent.aStaticActor->getGlobalPose();
	}

	if (!runPhysics)
	{
		physicsComponent.trans.p.x = pos.x;
		physicsComponent.trans.p.y = pos.y;
		physicsComponent.trans.p.z = pos.z;
	}

	//Lamda to handle axis rotation
	auto axisRotation = [this]()
	{

			float angleX = physicsComponent.physics_rot.x;
			float angleY = physicsComponent.physics_rot.y;
			float angleZ = physicsComponent.physics_rot.z;

			float angleXRad = physx::PxPi * angleX / 180.0f;
			float angleYRad = physx::PxPi * angleY / 180.0f;
			float angleZRad = physx::PxPi * angleZ / 180.0f;

			physx::PxQuat quatX(angleXRad, physx::PxVec3(1.0f, 0.0f, 0.0f));
			physx::PxQuat quatY(angleYRad, physx::PxVec3(0.0f, 1.0f, 0.0f));
			physx::PxQuat quatZ(angleZRad, physx::PxVec3(0.0f, 0.0f, 1.0f));

			physicsComponent.trans.q = quatX * quatY * quatZ;
	};

	if (!runPhysics && (physicsComponent.aActor || physicsComponent.aStaticActor))
	{
		axisRotation();
	}

	if (physicsComponent.aActor)
	{
		physicsComponent.aActor->setGlobalPose(physicsComponent.trans);
	}
	else if (physicsComponent.aStaticActor)
	{
		physicsComponent.aStaticActor->setGlobalPose(physicsComponent.trans);
	}

	if(runPhysics && (physicsComponent.aActor || physicsComponent.aStaticActor || physicsComponent.isCharacter))
		pos = DirectX::XMFLOAT3(physicsComponent.trans.p.x, physicsComponent.trans.p.y, physicsComponent.trans.p.z);
}

void Entity::Update(bool& runPhysics)
{
	UpdatePhysics(runPhysics);
}

void Entity::Draw(ID3D11DeviceContext* deviceContext, Camera& camera, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, float screenFrustumDepth, Texture* text, bool bCheckFrustum)
{

	DirectX::XMMATRIX matrix_scale;
	DirectX::XMMATRIX matrix_rotate;
	DirectX::XMMATRIX matrix_translate;

	if (physicsComponent.aActor || physicsComponent.aStaticActor)
	{
		if (physicsComponent.aActor)
			physicsComponent.trans = physicsComponent.aActor->getGlobalPose();
		else if (physicsComponent.aStaticActor)
			physicsComponent.trans = physicsComponent.aStaticActor->getGlobalPose();

		if (!model.isAttached)
		{
			matrix_scale = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
			matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
			matrix_rotate *= DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ physicsComponent.trans.q.x,physicsComponent.trans.q.y, physicsComponent.trans.q.z }, physicsComponent.trans.q.getAngle());

			matrix_translate = DirectX::XMMatrixTranslation(physicsComponent.trans.p.x + modelPos.x, physicsComponent.trans.p.y + modelPos.y, physicsComponent.trans.p.z + modelPos.z);
			worldMatrix = matrix_scale * matrix_rotate * matrix_translate;
		}
		else
		{
			if (parent)
			{

				DirectX::XMFLOAT3 floatPos;
				DirectX::XMFLOAT4 floatRot;
				DirectX::XMStoreFloat3(&floatPos, _pos);
				DirectX::XMStoreFloat4(&floatRot, _rot);


				physicsComponent.trans.p = physx::PxVec3(floatPos.x, floatPos.y, floatPos.z);
				physicsComponent.trans.q = physx::PxQuat(floatRot.x, floatRot.y, floatRot.z, floatRot.w);

				
				if (physicsComponent.aActor)
				{
					physicsComponent.aActor->setGlobalPose(physicsComponent.trans);
					//physicsComponent.aActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true)
					physicsComponent.aActor->getShapes(&physicsComponent.aShape, physicsComponent.aActor->getNbShapes());
					physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
					physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				}
				else if (physicsComponent.aStaticActor)
				{
					physicsComponent.aStaticActor->setGlobalPose(physicsComponent.trans);
					//physicsComponent.aStaticActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
					physicsComponent.aStaticActor->getShapes(&physicsComponent.aShape, physicsComponent.aStaticActor->getNbShapes());
					physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
					physicsComponent.aShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				}


				matrix_scale = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
				matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
				matrix_rotate *= DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ physicsComponent.trans.q.x,physicsComponent.trans.q.y, physicsComponent.trans.q.z }, physicsComponent.trans.q.getAngle());

				matrix_translate = DirectX::XMMatrixTranslation(physicsComponent.trans.p.x + modelPos.x, physicsComponent.trans.p.y + modelPos.y, physicsComponent.trans.p.z + modelPos.z);
				worldMatrix = matrix_scale * matrix_rotate * matrix_translate;
			}

		}
		//pos = DirectX::XMFLOAT3(physicsComponent.trans.p.x + modelPos.x, physicsComponent.trans.p.y + modelPos.y, physicsComponent.trans.p.z + modelPos.z);
	}
	else
	{

		matrix_scale = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
		matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
		matrix_translate = DirectX::XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);

		worldMatrix = matrix_scale * matrix_rotate * matrix_translate;
		if (!model.isAttached)
		{
			matrix_scale = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
			matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
			matrix_translate = DirectX::XMMatrixTranslation(pos.x + modelPos.x, pos.y + modelPos.y, pos.z + modelPos.z);



			worldMatrix = matrix_scale * matrix_rotate * matrix_translate;
		}
		else
		{
			matrix_scale = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
			matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
			matrix_rotate *= DirectX::XMMatrixRotationQuaternion(_rot);
			matrix_translate = DirectX::XMMatrixTranslationFromVector(_pos);
			worldMatrix = matrix_scale * matrix_rotate * matrix_translate;
		}
	}

	if (bRender)
	{

		if (!bCheckFrustum)
		{
			if (isAnimated)
				model.Update();
			model.Draw(deviceContext, worldMatrix, viewMatrix, projectionMatrix, text);
		}
		else
		{
			if (isfrustumEnabled)
			{

				DirectX::XMMATRIX view = viewMatrix;
				DirectX::XMMATRIX proj = projectionMatrix;
				frustum.ConstructFrustum(screenFrustumDepth, view, proj);

				if (physicsComponent.aActor || physicsComponent.aStaticActor)
					frustum.checkFrustum = frustum.CheckRect(physicsComponent.trans.p.x, physicsComponent.trans.p.y, physicsComponent.trans.p.z, scale.x + frustumScale.x, scale.y + frustumScale.y, scale.z + frustumScale.z);
				else
					frustum.checkFrustum = frustum.CheckRect(pos.x, pos.y, pos.z, scale.x + frustumScale.x, scale.y + frustumScale.y, scale.z + frustumScale.z);

				if (frustum.checkFrustum)
				{
					if (isAnimated)
						model.Update();
					model.Draw(deviceContext, worldMatrix, viewMatrix, projectionMatrix, text);
				}
			}
			else
			{
				if (isAnimated)
					model.Update();
				model.Draw(deviceContext, worldMatrix, viewMatrix, projectionMatrix, text);
			}
		}

	}

}

void Entity::FrustumDraw(Camera& camera, ID3D11DeviceContext* deviceContex, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, bool bCheckFrustum)
{
	frustum.frustumCube.scale = DirectX::XMFLOAT3(scale.x + frustumScale.x, scale.y + frustumScale.y, scale.z + frustumScale.z);
	frustum.frustumCube.pos = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);

	frustum.frustumCube.Draw(deviceContex, camera, cb_vs_vertexshader);
}

void Entity::AttachController(physx::PxController& characterController, bool& runPhysics)
{
	if (isDeleted)
		return;

	physicsComponent.trans = characterController.getActor()->getGlobalPose();
	//pos = DirectX::XMFLOAT3(characterController.getPosition().x, characterController.getPosition().y, characterController.getPosition().z);


	if (!runPhysics)
	{
		characterController.setPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
	}

	physicsComponent.trans.q = physx::PxQuat(physicsComponent.physics_rot.w, physx::PxVec3(physicsComponent.physics_rot.x, physicsComponent.physics_rot.y, physicsComponent.physics_rot.z));


	physicsComponent.trans.q = physicsComponent.trans.q.getNormalized();


	characterController.getActor()->setGlobalPose(physicsComponent.trans);




	matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	matrix_rotate *= DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ physicsComponent.trans.q.x,physicsComponent.trans.q.y, physicsComponent.trans.q.z }, physicsComponent.trans.q.getAngle());

}

void Entity::SetupAttachment(Entity* entity)
{

	if (entity)
		parent = entity;
	else
		return;

	if(parent->attachedEntities.size() == 0)
		parent->attachedEntities.push_back(this);
	else
	{
		bool alreadyAttached = false;
		for (Entity* e : parent->attachedEntities) {
			if (e == this || e->entityName == this->entityName) {
				alreadyAttached = true;
				break;
			}
		}

		if (!alreadyAttached)
			parent->attachedEntities.push_back(this);
	}

	if (model.isAttached && parent)
	{
		DirectX::XMMATRIX boneTrans;
		parent->model.AttachTo(attachedBone, boneTrans);
		boneTrans = boneTrans * parent->worldMatrix;


		DirectX::XMMatrixDecompose(&_scale, &_rot, &_pos, boneTrans);

		DirectX::XMStoreFloat3(&parent->model.worldPos, _pos);
		DirectX::XMStoreFloat3(&parent->model.worldScale, _scale);
		DirectX::XMStoreFloat4(&parent->model.worldRot, _rot);

		pos = DirectX::XMFLOAT3(parent->model.worldPos.x, parent->model.worldPos.y, parent->model.worldPos.z);
	}
}
void Entity::DrawGui(physx::PxScene& scene, std::vector<std::shared_ptr<Entity>>& entities, int index)
{
	if (isDeleted)
		return;

	ImGui::Text(entityName.c_str());
	ImGui::SameLine();
	if (ImGui::Button(ImGuiIDLabel("Delete##", index).c_str()))
	{
		bFlagForDeletion = true;
	}
	ImGui::Text((ImGuiIDLabel("X: ##",index).c_str() + std::to_string(physicsComponent.trans.p.x)).c_str());
	ImGui::SameLine();
	ImGui::Text((ImGuiIDLabel(" Y: ##",index).c_str() + std::to_string(physicsComponent.trans.p.y)).c_str());
	ImGui::SameLine();
	ImGui::Text((ImGuiIDLabel(" Z: ##",index).c_str() + std::to_string(physicsComponent.trans.p.z)).c_str());
	ImGui::DragFloat3(ImGuiIDLabel("pos##",index).c_str(), &pos.x, 0.01f);
	if (physicsComponent.aActor || physicsComponent.aStaticActor || physicsComponent.isCharacter)
	{
		ImGui::DragFloat3(ImGuiIDLabel("scale##",index).c_str(), &physicsComponent.physics_scale.x, 0.01f, 0.1f);
		ImGui::DragFloat4(ImGuiIDLabel("rot##",index).c_str(), &physicsComponent.physics_rot.x, 0.5f);
	}
	else
	{
		ImGui::DragFloat3(ImGuiIDLabel("rot##",index).c_str(), &rot.x, 0.01f);
		ImGui::DragFloat3(ImGuiIDLabel("scale##",index).c_str(), &scale.x, 0.01f);
	}

	static bool showHidden = false;
	ImGui::Checkbox(ImGuiIDLabel("More##", index).c_str(), &showHidden);

	if (showHidden)
	{

		if (ImGui::CollapsingHeader(ImGuiIDLabel("Model transform##", index).c_str()))
		{
			ImGui::DragFloat3(ImGuiIDLabel("modelPos##",index).c_str(), &modelPos.x, 0.01f);

			if (physicsComponent.aActor || physicsComponent.aStaticActor || physicsComponent.isCharacter)
			{
				ImGui::DragFloat3(ImGuiIDLabel("model_rotation##",index).c_str(), &rot.x, 0.01f);
				ImGui::DragFloat3(ImGuiIDLabel("model_scale##",index).c_str(), &scale.x, 0.01f);
			}
		}

		if (ImGui::CollapsingHeader(ImGuiIDLabel("Options##",index).c_str()))
		{
			ImGui::Checkbox(ImGuiIDLabel("isCharacter##",index).c_str(), &physicsComponent.isCharacter);
			ImGui::Checkbox(ImGuiIDLabel("isPlayer##", index).c_str(), &isPlayer);
			ImGui::Checkbox(ImGuiIDLabel("isAI##", index).c_str(), &isAI);
			ImGui::Checkbox(ImGuiIDLabel("isWalkable##",index).c_str(), &isWalkable);
			ImGui::Checkbox(ImGuiIDLabel("isObstacle##",index).c_str(), &isObstacle);
			ImGui::Checkbox(ImGuiIDLabel("isAttached##",index).c_str(), &model.isAttached);
			ImGui::Checkbox(ImGuiIDLabel("isEmissive##",index).c_str(), &isEmissive);
			if (ImGui::Button(ImGuiIDLabel("Create Controller##", index).c_str()))
			{
				bCreateController = true;
			}
			ImGui::Checkbox(ImGuiIDLabel("Render##",index).c_str(), &bRender);
		}
	

		if (ImGui::CollapsingHeader(ImGuiIDLabel("Extra##",index).c_str()))
		{
			ImGui::DragFloat3(ImGuiIDLabel("frustumScale##",index).c_str(), &frustumScale.x, 0.01f);
			ImGui::DragFloat3(ImGuiIDLabel("emissiveColor##",index).c_str(), &emissiveColor.x, 0.01f);
			ImGui::DragFloat3(ImGuiIDLabel("BoneRot##",index).c_str(), &model.BoneRot.x, 0.01f);

			ImGui::Checkbox(ImGuiIDLabel("isTransparent##",index).c_str(), &model.isTransparent);
			ImGui::Checkbox(ImGuiIDLabel("Frustum##",index).c_str(), &isfrustumEnabled);
		}

		if (ImGui::CollapsingHeader(ImGuiIDLabel("Physics##",index).c_str()))
		{
			ImGui::DragFloat(ImGuiIDLabel("Mass##",index).c_str(), &physicsComponent.mass);
			ImGui::InputInt(ImGuiIDLabel("triangleMeshStride##",index).c_str(), &physicsComponent.triangleMeshStride);
			ImGui::InputInt(ImGuiIDLabel("convexMeshDetail##",index).c_str(), &physicsComponent.convexMeshDetail);
			ImGui::InputInt(ImGuiIDLabel("Physics Shape##",index).c_str(), &physicsComponent.selectedShape);
		}

		if (ImGui::Button(ImGuiIDLabel("Apply##",index).c_str()))
		{
			physicsComponent.physicsShapeEnum = static_cast<PhysicsShapeEnum>(physicsComponent.selectedShape);
			physicsComponent.bCreatePhysicsComp = true;
		}



		if (isAnimated)
		{
			std::fstream f;

			bool open = false, save = false;

			//ImGui::SameLine();
			if (ImGui::BeginMenu(ImGuiIDLabel("Anim Files##",index).c_str()))
			{
				if (ImGui::MenuItem(ImGuiIDLabel("Open##",index).c_str(), NULL))
					open = true;

				ImGui::EndMenu();
			}

			//Remember the name to ImGui::OpenPopup() and showFileDialog() must be same...
			if (open)
				ImGui::OpenPopup(ImGuiIDLabel("Open File##",index).c_str());

			if (file_dialog.showFileDialog(ImGuiIDLabel("Open File##",index).c_str(), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(400, 200), "*.*,.obj,.dae,.gltf,.glb,.fbx"))
			{
				std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
				std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
				f = std::fstream(file_dialog.selected_path.c_str());
				if (f.good())
					inName = file_dialog.selected_path;


			}
			if (ImGui::Button(ImGuiIDLabel("AddAnim##",index).c_str()))
			{
				model.LoadAnimation(inName);
				model.animFiles.push_back(inName);
			}


		}
		if (model.isAttached)
		{
			if (ImGui::CollapsingHeader(ImGuiIDLabel("Show entities##",index).c_str()))
			{
				std::vector<const char*> _entitiesData;
				for (int i = 0; i < entities.size(); ++i)
				{
					if (entities[i]->isAnimated)
						_entitiesData.push_back(entities[i]->entityName.c_str());
				}
				static int listbox_current = -1;
				ImGui::ListBox(ImGuiIDLabel("Entities##",index).c_str(), &listbox_current, _entitiesData.data(), _entitiesData.size());

				if (listbox_current > -1)
				{
					parentName = _entitiesData[listbox_current];
					for (int i = 0; i < entities.size(); ++i)
					{
						if (entities[i]->entityName == _entitiesData[listbox_current])
						{
							parent = entities[i].get();
						}
					}
				}


			}

			if (ImGui::CollapsingHeader(ImGuiIDLabel("Show skeleton##",index).c_str()))
			{
				if (parent)
				{
					std::vector<const char*> _bonesData;
					for (int i = 0; i < parent->model.boneNames.size(); ++i)
					{
						_bonesData.push_back(parent->model.boneNames[i].c_str());
					}
					static int listbox_current = 0;
					ImGui::ListBox(ImGuiIDLabel("Skeleton##",index).c_str(), &listbox_current, _bonesData.data(), _bonesData.size());

					if (listbox_current > -1)
						attachedBone = _bonesData[listbox_current];
				}
			}
		}

		if (isAnimated)
		{
			if (ImGui::CollapsingHeader(ImGuiIDLabel("Show skeleton##",index).c_str()))
			{
				std::vector<const char*> _bonesData;
				for (int i = 0; i < model.boneNames.size(); ++i)
				{
					_bonesData.push_back(model.boneNames[i].c_str());
				}
				static int listbox_current = 0;
				ImGui::ListBox(ImGuiIDLabel("Skeleton##",index).c_str(), &listbox_current, _bonesData.data(), _bonesData.size());
			}
		}

		if (ImGui::CollapsingHeader(ImGuiIDLabel("Show attachments##",index).c_str()))
		{

			std::vector<const char*> attachmentsData;
			for (int i = 0; i < attachedEntities.size(); ++i)
			{
				attachmentsData.push_back(attachedEntities[i]->entityName.c_str());
			}
			static int listbox_current = -1;
			ImGui::ListBox(ImGuiIDLabel("Entities##",index).c_str(), &listbox_current, attachmentsData.data(), attachmentsData.size());

			if (listbox_current > -1)
			{
				for (int i = 0; i < attachedEntities.size(); ++i)
				{

					if (attachedEntities[i]->entityName == attachmentsData[listbox_current])
					{
						if (attachedEntities[i]->physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							attachedEntities[i]->physicsComponent.aActor->getShapes(&_shape, attachedEntities[i]->physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);


						}
						else if (attachedEntities[i]->physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							attachedEntities[i]->physicsComponent.aStaticActor->getShapes(&_shape, attachedEntities[i]->physicsComponent.aStaticActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
						}
						attachedEntities[i]->DrawGui(scene, entities, i+1);
					}
					else
					{
						if (attachedEntities[i]->physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							attachedEntities[i]->physicsComponent.aActor->getShapes(&_shape, attachedEntities[i]->physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
						}
						else if (attachedEntities[i]->physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							attachedEntities[i]->physicsComponent.aStaticActor->getShapes(&_shape, attachedEntities[i]->physicsComponent.aStaticActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
						}
					}
				}
			}
		}
	}
}

void Entity::Input(Mouse& mouse, Keyboard& keyboard)
{
}


void Entity::Clear(physx::PxScene& scene)
{
	//if (isDeleted)
		//return;

	isDeleted = true;
	model.Clear();
	if (physicsComponent.aStaticActor)
	{

		scene.removeActor(*physicsComponent.aStaticActor);
		physicsComponent.aStaticActor->release();
	}
	else if (physicsComponent.aActor)
	{
		scene.removeActor(*physicsComponent.aActor);
		physicsComponent.aActor->release();
	}
}
