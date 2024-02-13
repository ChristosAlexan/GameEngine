#pragma once
#include "Entity.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "SoundComponent.h"
#include "AppTimer.h"
#include "Controller.h"

class TpsController : public Controller
{
public:
	TpsController();

	void Intitialize(ID3D11Device* device);
	void MouseMovement(float& dt, Entity& entity, Keyboard& keyboard, Mouse& mouse, Camera& camera);
	void Movement(float& dt, float gravity, Entity& entity, Keyboard& keyboard, Mouse& mouse, Camera& camera);
	void SetCharacterRotation(Entity& entity, Camera& camera);
	void Actions(Keyboard& keyboard, Mouse& mouse, Camera& camera);
	void UpdateSounds(Camera& camera, Entity* player);

public:
	bool isFiring = false;

	SoundComponent rifleFireSound;

private:

	RotationEnum currRotation;
	RotationEnum prevRotation;

	bool rotateLeft = false;
	bool rotateUp = false;
	bool rotateRight = false;

	AppTimer timer;
	double val = 0.0;
	bool canJump = true;
	bool isJumping = false;
	bool jumpkeyIsPressed = false;
	
	AppTimer fireTimer;

private:
	bool bCanFire;
	bool canPressSpace = true;
	float CharacterRotY = 0.0f;
	float CharacterRotX = 0.0f;

	float lastCamRot = 0.0f;
	bool isMoving = false;
	bool isFalling = false;
	bool isAiming = false;
	DirectX::XMVECTOR vLookAt;



	float zoom = 3.4f;
};

