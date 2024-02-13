#include "TpsController.h"
#include <algorithm>

TpsController::TpsController()
{
	isJumping = false;
	timer.Start();
	currRotation = RotationEnum::UP;
	vLookAt = DirectX::XMVECTOR{ 0,0,0 };
	CharacterRotY = 0.0f;
	zoom = 2.3f;
	bCanFire = true;
}

void TpsController::Intitialize(ID3D11Device* device)
{
	rifleFireSound.Initialize("./Data/Sounds/RifleFire.wav",true, device);
}

void TpsController::MouseMovement(float& dt, Entity& entity, Keyboard& keyboard, Mouse& mouse, Camera& camera)
{
	float cameraSpeed;

	if (isAiming)
	{
		cameraSpeed = 0.01f;
	}
	else
	{
		cameraSpeed = 0.03f;
	}
	DirectX::XMFLOAT4 rightFloat4;
	DirectX::XMStoreFloat4(&rightFloat4, camera.GetRightVector());
	DirectX::XMFLOAT4 forwardFloat4;
	DirectX::XMStoreFloat4(&forwardFloat4, camera.GetForwardVector());

	if (camera.PossessCharacter)
	{
		entity.physicsComponent.trans = entity.physicsComponent.aActor->getGlobalPose();
		entity.pos = DirectX::XMFLOAT3(entity.physicsComponent.trans.p.x + (0.32f * rightFloat4.x), entity.physicsComponent.trans.p.y, entity.physicsComponent.trans.p.z + (0.32f * rightFloat4.z));



		while (!mouse.EventBufferIsEmpty())
		{
			MouseEvent me = mouse.ReadEvent();

			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				CharacterRotY += static_cast<float>(me.GetPosY()) * cameraSpeed * 0.15f;
				CharacterRotY = std::clamp(CharacterRotY, -2.5f, 2.5f);

				CharacterRotX = me.GetPosX();
			
				camera.AdjustPosition(rightFloat4.x * -cameraSpeed * static_cast<float>(me.GetPosX() * 0.1f), 0, rightFloat4.z * -cameraSpeed * static_cast<float>(me.GetPosX() * 0.1f));
			}

		}

		DirectX::XMFLOAT3 _finalLookAt;
		DirectX::XMStoreFloat3(&_finalLookAt, vLookAt);
		camera.SetLookAtPos(DirectX::XMFLOAT3(_finalLookAt.x, _finalLookAt.y, _finalLookAt.z));
		vLookAt = DirectX::XMVECTOR{ entity.pos.x, entity.pos.y + 0.7f, entity.pos.z };

		if (mouse.IsRightDown())
		{
			zoom -= 0.008f * dt;
		}
		else
		{
			zoom += 0.008f * dt;
		}
		zoom = std::clamp(zoom, 0.8f, 2.0f);

		camera.SetPosition(entity.pos.x + (-zoom * std::sin(camera.yaw)), entity.pos.y + CharacterRotY, entity.pos.z + (-zoom * std::cos(camera.yaw)));
	}
}

void TpsController::Movement(float& dt, float gravity, Entity& entity, Keyboard& keyboard, Mouse& mouse, Camera& camera)
{

	if (entity.isDeleted)
		return;
	if (!entity.physicsComponent.aActor)
		return;


	isFalling = entity.isFalling;
	entity.isMovingLeft = false;

	DirectX::XMFLOAT4 forwardDir;
	DirectX::XMStoreFloat4(&forwardDir, camera.vec_forward);
	DirectX::XMFLOAT4 rightDir;
	DirectX::XMStoreFloat4(&rightDir, camera.vec_right);


	float velocity = 5.0;
	float moveX = 0.0f;
	float moveZ = 0.0f;

	entity.physicsComponent.trans = entity.physicsComponent.aActor->getGlobalPose();

	if (mouse.IsRightDown())
	{
		isAiming = true;
		//Rotate with camera
		entity.physicsComponent.trans.q = physx::PxQuat((camera.rot.y), physx::PxVec3(0, 1, 0));
		currRotation = RotationEnum::UP;
	}
	else
	{
		isAiming = false;
		SetCharacterRotation(entity, camera);
		if (entity.isMovingFront || entity.isMovingLeft || entity.isMovingRight)
		{
			entity.physicsComponent.trans.q = physx::PxQuat((camera.rot.y + entity.rotationDir), physx::PxVec3(0, 1, 0));
			lastCamRot = camera.rot.y;
		}
		else
			entity.physicsComponent.trans.q = physx::PxQuat((lastCamRot + entity.rotationDir), physx::PxVec3(0, 1, 0));
	}
	//entity.physicsComponent.trans.q = physx::PxQuat((camera.rot.y + entity.rotationDir), physx::PxVec3(0, 1, 0));



	entity.physicsComponent.aActor->setGlobalPose(entity.physicsComponent.trans);

	entity.matrix_rotate = DirectX::XMMatrixRotationRollPitchYaw(entity.rot.x, entity.rot.y, entity.rot.z);

	//entity.matrix_rotate *= XMMatrixRotationAxis(XMVECTOR{ 0, 1, 0 }, camera.rot.y);
	//entity.matrix_rotate *= XMMatrixRotationAxis(XMVECTOR{ 0, entity.physicsComponent.trans.q.y, 0 }, entity.physicsComponent.trans.q.w);

	if (keyboard.KeyIsPressed(VK_F8))
	{
		camera.PossessCharacter = true;
	}
	if (keyboard.KeyIsPressed(VK_F9))
	{
		camera.PossessCharacter = false;
	}

	entity.pos = DirectX::XMFLOAT3(entity.physicsComponent.trans.p.x, entity.physicsComponent.trans.p.y, entity.physicsComponent.trans.p.z);
	if (camera.PossessCharacter)
	{

		if (keyboard.KeyIsPressed('S') && (keyboard.KeyIsPressed('A')))
		{
			currRotation = RotationEnum::LEFT_DOWN;
			entity.isMovingRight = true;
			entity.isMovingLeft = true;

			moveX = -velocity * forwardDir.x - velocity * rightDir.x;
			moveZ = -velocity * forwardDir.z - velocity * rightDir.z;



			entity.model.currAnim = 1;
		}
		else if (keyboard.KeyIsPressed('S') && (keyboard.KeyIsPressed('D')))
		{
			currRotation = RotationEnum::RIGHT_DOWN;
			entity.isMovingRight = true;

			moveX = -velocity * forwardDir.x + velocity * rightDir.x;
			moveZ = -velocity * forwardDir.z + velocity * rightDir.z;

			entity.model.currAnim = 1;
		}
		else if (keyboard.KeyIsPressed('W') && (keyboard.KeyIsPressed('A')))
		{
			currRotation = RotationEnum::LEFT_UP;
			entity.isMovingRight = true;
			entity.isMovingLeft = true;
			moveX = velocity * forwardDir.x - velocity * rightDir.x;
			moveZ = velocity * forwardDir.z - velocity * rightDir.z;


			entity.model.currAnim = 1;
		}
		else if (keyboard.KeyIsPressed('W') && (keyboard.KeyIsPressed('D')))
		{
			currRotation = RotationEnum::RIGHT_UP;
			entity.isMovingRight = true;

			moveX = velocity * forwardDir.x + velocity * rightDir.x;
			moveZ = velocity * forwardDir.z + velocity * rightDir.z;

			entity.model.currAnim = 1;
		}
		else
		{
			entity.isMovingRight = false;
			if (keyboard.KeyIsPressed('W'))
			{
				currRotation = RotationEnum::UP;
				entity.isMovingFront = true;

				moveX = velocity * forwardDir.x;
				moveZ = velocity * forwardDir.z;

			}
			else if (keyboard.KeyIsPressed('S'))
			{
				currRotation = RotationEnum::DOWN;
				entity.isMovingFront = true;

				moveX = -velocity * forwardDir.x;
				moveZ = -velocity * forwardDir.z;


				entity.model.currAnim = 1;

			}
			else
			{
				entity.isMovingFront = false;
			}


			if (keyboard.KeyIsPressed('D'))
			{

				currRotation = RotationEnum::RIGHT;
				entity.isMovingRight = true;

				moveX = velocity * rightDir.x;
				moveZ = velocity * rightDir.z;



			}
			else if (keyboard.KeyIsPressed('A'))
			{
				currRotation = RotationEnum::LEFT;
				entity.isMovingRight = true;
				entity.isMovingLeft = true;


				moveX = -velocity * rightDir.x;
				moveZ = -velocity * rightDir.z;


				entity.model.currAnim = 1;

			}
			else
			{
				entity.isMovingRight = false;
			}


			if ((!entity.isMovingFront && !entity.isMovingRight) || entity.isFalling)
			{
				entity.model.currAnim = 0;



			}
		}

		if (!entity.isFalling)
		{
			if (canPressSpace)
			{
				if (keyboard.KeyIsPressed(VK_SPACE))
				{
					canPressSpace = false;
					timer.Restart();
					isJumping = true;
					entity.physicsComponent.aActor->addForce(physx::PxVec3(moveX, 350.0f, moveZ), physx::PxForceMode::eIMPULSE);
				}
			}

			if (!keyboard.KeyIsPressed(VK_SPACE))
			{
				canPressSpace = true;
			}
		}


		if (timer.GetMilisecondsElapsed() > 2.0f * dt)
		{
			if (!entity.isFalling && isJumping)
			{
				isJumping = false;
			}
		}
		if (!isJumping)
		{
			if (!entity.isFalling)
			{
				isJumping = false;
				entity.physicsComponent.aActor->setLinearVelocity(physx::PxVec3(moveX, 0.0f, moveZ));
			}
		}

		if (entity.isFalling)
		{
			entity.physicsComponent.aActor->addForce(physx::PxVec3(moveX * 30, 0, moveZ * 30), physx::PxForceMode::eFORCE);
		}

		if (isAiming)
		{
			entity.model.SetAnimIndex(3, true, 2.0f);
		}
		else
		{
			if (entity.isFalling)
			{
				entity.model.SetAnimIndex(2, true, 8.0f);
			}
			else
			{
				if (moveX != 0 || moveZ != 0)
				{
					isMoving = true;
					entity.model.SetAnimIndex(1, true, 5.0f);
				}
				else
				{
					isMoving = false;
					entity.model.SetAnimIndex(0, true, 5.0f);
				}
			}
		}
	

		entity.physicsComponent.aActor->setGlobalPose(entity.physicsComponent.trans);
	}
}



void TpsController::SetCharacterRotation(Entity& entity, Camera& camera)
{
	const float PI = 3.14f;
	const float HALF_PI = PI / 2.0f;

	float rotSpeed = 0.15f;
	switch (currRotation)
	{
	case UP:
		entity.rotationDir = 0.0f;  // Facing forward
		break;
	case DOWN:
		entity.rotationDir = PI;  // Facing backward
		break;
	case RIGHT:
		entity.rotationDir = HALF_PI;  // Facing right
		break;
	case LEFT:
		entity.rotationDir = -HALF_PI;  // Facing left
		break;
	case RIGHT_UP:
		entity.rotationDir = HALF_PI / 2.0f;  // Facing right-up
		break;
	case RIGHT_DOWN:
		entity.rotationDir = 3.0f * HALF_PI / 2.0f;  // Facing right-down
		break;
	case LEFT_UP:
		entity.rotationDir = -HALF_PI / 2.0f;  // Facing left-up
		break;
	case LEFT_DOWN:
		entity.rotationDir = -3.0f * HALF_PI / 2.0f;  // Facing left-down
		break;
	}
}

void TpsController::Actions(Keyboard& keyboard, Mouse& mouse, Camera& camera)
{
	if (bCanFire)
	{
		fireTimer.Stop();
		fireTimer.Restart();
		if (mouse.IsLeftDown() && mouse.IsRightDown())
		{
			bCanFire = false;
			isFiring = true;
		}
		else
		{
			isFiring = false;
		}
		if (isFiring)
			fireTimer.Start();
	}
	else
		isFiring = false;
	
	if (!bCanFire)
	{
		if (fireTimer.GetMilisecondsElapsed() > 200.0f)
		{
			bCanFire = true;
			fireTimer.Restart();
		}
	}
	
	//OutputDebugStringA(("TIMER = " + std::to_string(fireTimer.GetMilisecondsElapsed())).c_str());
	//OutputDebugStringA("\n");
}


void TpsController::UpdateSounds(Camera& camera, Entity* player)
{
	if (isFiring)
	{
		if (player)
			rifleFireSound.cube.pos = player->pos;
		rifleFireSound.UpdatePos(camera.GetPositionFloat3(), camera.GetForwardVector(), camera.upDir);
		rifleFireSound.Async_Update();
		rifleFireSound.Async_Play();
	}
}
