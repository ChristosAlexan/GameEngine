#pragma once

#include <PhysX/PxPhysicsAPI.h>

class Controller
{
public:
	Controller();
	physx::PxVec3 GetForwardVec(physx::PxQuat& _rot, physx::PxVec3& _pos);

protected:
	enum RotationEnum
	{
		UP = 0,
		RIGHT_UP = 1,
		RIGHT = 2,
		RIGHT_DOWN = 3,
		DOWN = 4,
		LEFT_DOWN = 5,
		LEFT = 6,
		LEFT_UP = 7,
	};
};

