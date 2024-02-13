#pragma once
#include "Entity.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "Controller.h"

class FpsController : public Controller
{
public:
	FpsController();

	void MouseMovement(float& dt, Entity& entity, Keyboard& keyboard, Mouse& mouse, Camera& camera);
	void Movement(float& dt, float gravity, Entity* entity, Keyboard& keyboard, Mouse& mouse, Camera& camera);

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

private:
	bool canPressSpace = true;
};

