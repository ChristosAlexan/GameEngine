#include "Entity.h"

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
		physicsComponent.CreateTriangleMesh(physics, scene, model.m_vertices, model.m_indices, physx::PxVec3(scale.x, scale.y, scale.z), physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(1, physx::PxVec3(rot.x, rot.y, rot.z)));
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

void Entity::Draw(Camera& camera, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, Texture* text, bool bCheckFrustum)
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
			model.Draw(worldMatrix, viewMatrix, projectionMatrix, text);
		}
		else
		{
			if (isfrustumEnabled)
			{

				DirectX::XMMATRIX view = viewMatrix;
				DirectX::XMMATRIX proj = projectionMatrix;
				frustum.ConstructFrustum(100, view, proj);

				if (physicsComponent.aActor || physicsComponent.aStaticActor)
					frustum.checkFrustum = frustum.CheckRect(physicsComponent.trans.p.x, physicsComponent.trans.p.y, physicsComponent.trans.p.z, scale.x + frustumScale.x, scale.y + frustumScale.y, scale.z + frustumScale.z);
				else
					frustum.checkFrustum = frustum.CheckRect(pos.x, pos.y, pos.z, scale.x + frustumScale.x, scale.y + frustumScale.y, scale.z + frustumScale.z);

				if (frustum.checkFrustum)
				{
					if (isAnimated)
						model.Update();
					model.Draw(worldMatrix, viewMatrix, projectionMatrix, text);
				}
			}
			else
			{
				if (isAnimated)
					model.Update();
				model.Draw(worldMatrix, viewMatrix, projectionMatrix, text);
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
void Entity::DrawGui(physx::PxScene& scene, std::vector<Entity>& entities)
{
	if (isDeleted)
		return;


	ImGui::Text(entityName.c_str());
	ImGui::SameLine();

	if (ImGui::Button("Delete"))
	{
		bFlagForDeletion = true;
	}
	ImGui::Text(("X: " + std::to_string(physicsComponent.trans.p.x)).c_str());
	ImGui::SameLine();
	ImGui::Text((" Y: " + std::to_string(physicsComponent.trans.p.y)).c_str());
	ImGui::SameLine();
	ImGui::Text((" Z: " + std::to_string(physicsComponent.trans.p.z)).c_str());
	ImGui::DragFloat3("pos", &pos.x, 0.01f);
	if (physicsComponent.aActor || physicsComponent.aStaticActor || physicsComponent.isCharacter)
	{
		ImGui::DragFloat3("scale", &physicsComponent.physics_scale.x, 0.01f);
		ImGui::DragFloat4("rot", &physicsComponent.physics_rot.x, 0.5f);
	}
	else
	{
		ImGui::DragFloat3("rot", &rot.x, 0.01f);
		ImGui::DragFloat3("scale", &scale.x, 0.01f);
	}

	static bool showHidden = false;
	ImGui::Checkbox("More", &showHidden);

	if (showHidden)
	{

		if (ImGui::CollapsingHeader("Model transform"))
		{
			ImGui::DragFloat3("modelPos", &modelPos.x, 0.01f);

			if (physicsComponent.aActor || physicsComponent.aStaticActor || physicsComponent.isCharacter)
			{
				ImGui::DragFloat3("model_rotation", &rot.x, 0.01f);
				ImGui::DragFloat3("model_scale", &scale.x, 0.01f);
			}
		}

		if (ImGui::CollapsingHeader("Options"))
		{
			ImGui::Checkbox("isCharacter", &physicsComponent.isCharacter);
			ImGui::Checkbox("isPlayer", &isPlayer);
			ImGui::Checkbox("isAI", &isAI);
			ImGui::Checkbox("isWalkable", &isWalkable);
			ImGui::Checkbox("isObstacle", &isObstacle);
			ImGui::Checkbox("isAttached", &model.isAttached);
			ImGui::Checkbox("isEmissive", &isEmissive);
			if (ImGui::Button("Create Controller"))
			{
				bCreateController = true;
			}
			ImGui::Checkbox("Render", &bRender);
		}
	

		if (ImGui::CollapsingHeader("Extra"))
		{
			ImGui::DragFloat3("frustumScale", &frustumScale.x, 0.01f);
			ImGui::DragFloat3("emissiveColor", &emissiveColor.x, 0.01f);
			ImGui::DragFloat3("BoneRot", &model.BoneRot.x, 0.01f);

			ImGui::Checkbox("isTransparent", &model.isTransparent);
			ImGui::Checkbox("Frustum", &isfrustumEnabled);
		}

		if (ImGui::CollapsingHeader("Physics"))
		{
			ImGui::DragFloat("Mass", &physicsComponent.mass);
			ImGui::InputInt("triangleMeshStride", &physicsComponent.triangleMeshStride);
			ImGui::InputInt("convexMeshDetail", &physicsComponent.convexMeshDetail);
			ImGui::InputInt("Physics Shape", &physicsComponent.selectedShape);
		}

		if (ImGui::Button("Apply"))
		{
			physicsComponent.physicsShapeEnum = static_cast<PhysicsShapeEnum>(physicsComponent.selectedShape);
			physicsComponent.bCreatePhysicsComp = true;
		}



		if (isAnimated)
		{
			std::fstream f;

			bool open = false, save = false;

			//ImGui::SameLine();
			if (ImGui::BeginMenu("Anim Files"))
			{
				if (ImGui::MenuItem("Open", NULL))
					open = true;

				ImGui::EndMenu();
			}

			//Remember the name to ImGui::OpenPopup() and showFileDialog() must be same...
			if (open)
				ImGui::OpenPopup("Open File");

			if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(400, 200), "*.*,.obj,.dae,.gltf,.glb,.fbx"))
			{
				std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
				std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
				f = std::fstream(file_dialog.selected_path.c_str());
				if (f.good())
					inName = file_dialog.selected_path;


			}
			if (ImGui::Button("AddAnim"))
			{
				model.LoadAnimation(inName);
				model.animFiles.push_back(inName);
			}


		}
		if (model.isAttached)
		{
			if (ImGui::CollapsingHeader("Show entities"))
			{
				std::vector<const char*> _entitiesData;
				for (int i = 0; i < entities.size(); ++i)
				{
					if (entities[i].isAnimated)
						_entitiesData.push_back(entities[i].entityName.c_str());
				}
				static int listbox_current = -1;
				ImGui::ListBox("Entities", &listbox_current, _entitiesData.data(), _entitiesData.size());

				if (listbox_current > -1)
				{
					parentName = _entitiesData[listbox_current];
					for (int i = 0; i < entities.size(); ++i)
					{
						if (entities[i].entityName == _entitiesData[listbox_current])
						{
							parent = &entities[i];
						}
					}
				}


			}

			if (ImGui::CollapsingHeader("Show skeleton"))
			{
				if (parent)
				{
					std::vector<const char*> _bonesData;
					for (int i = 0; i < parent->model.boneNames.size(); ++i)
					{
						_bonesData.push_back(parent->model.boneNames[i].c_str());
					}
					static int listbox_current = 0;
					ImGui::ListBox("Skeleton", &listbox_current, _bonesData.data(), _bonesData.size());

					if (listbox_current > -1)
						attachedBone = _bonesData[listbox_current];
				}
			}
		}

		if (isAnimated)
		{
			if (ImGui::CollapsingHeader("Show skeleton"))
			{
				std::vector<const char*> _bonesData;
				for (int i = 0; i < model.boneNames.size(); ++i)
				{
					_bonesData.push_back(model.boneNames[i].c_str());
				}
				static int listbox_current = 0;
				ImGui::ListBox("Skeleton", &listbox_current, _bonesData.data(), _bonesData.size());
			}
		}
	}
}

void Entity::Input(Mouse& mouse, Keyboard& keyboard)
{
}


void Entity::Clear(physx::PxScene& scene)
{
	if (isDeleted)
		return;

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