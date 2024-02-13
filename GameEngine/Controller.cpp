#include "Controller.h"

Controller::Controller()
{
}

physx::PxVec3 Controller::GetForwardVec(physx::PxQuat& _rot, physx::PxVec3& _pos)
{
	float num1 = _rot.x * 2.0f;
	float num2 = _rot.y * 2.0f;
	float num3 = _rot.z * 2.0f;
	float num4 = _rot.x * num1;
	float num5 = _rot.y * num2;
	float num6 = _rot.z * num3;
	float num7 = _rot.x * num2;
	float num8 = _rot.x * num3;
	float num9 = _rot.y * num3;
	float num10 = _rot.w * num1;
	float num11 = _rot.w * num2;
	float num12 = _rot.w * num3;

	physx::PxVec3 vector3;
	vector3.x = (float)((1.0 - ((double)num5 + (double)num6)) * (double)_pos.x + ((double)num7 - (double)num12) * (double)_pos.y + ((double)num8 + (double)num11) * (double)_pos.z);
	vector3.y = (float)(((double)num7 + (double)num12) * (double)_pos.x + (1.0 - ((double)num4 + (double)num6)) * (double)_pos.y + ((double)num9 - (double)num10) * (double)_pos.z);
	vector3.z = (float)(((double)num8 - (double)num11) * (double)_pos.x + ((double)num9 + (double)num10) * (double)_pos.y + (1.0 - ((double)num4 + (double)num5)) * (double)_pos.z);

	return vector3;
}