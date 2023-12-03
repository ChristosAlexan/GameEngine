#pragma once
#include <PhysX/PxPhysicsAPI.h>
#include <vector>
#include "Vertex.h"
#include <Windows.h>
#include <future>

enum PhysicsShapeEnum
{
	NONE = -1,
	CUBE = 0,
	PLANE = 1,
	SPHERE = 2,
	CAPSULE = 3,
	CONVEXMESH = 4,
	TRIANGLEMESH = 5
};
class PhysicsComponent
{
public:
	PhysicsComponent();
	~PhysicsComponent();

	void CreateCube(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxVec3 _scale, physx::PxVec3 _pos);
	void CreatePlane(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxVec3 _pos);
	void CreateSphere(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxReal radius, physx::PxVec3 _pos);
	void CreateCapsule(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxVec3 _scale, physx::PxVec3 _pos);
	void CreateConvex(physx::PxPhysics& physics, physx::PxScene& scene, std::vector<Vertex>& vertices, physx::PxVec3 _scale, physx::PxVec3 _pos);
	void CreateTriangleMesh(physx::PxPhysics& physics, physx::PxScene& scene, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, physx::PxVec3 _scale, physx::PxVec3 _pos);

	void CreatePickingShape(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxVec3 _scale, physx::PxVec3 _pos);

	void UpdatePhysics(physx::PxPhysics& physics, physx::PxScene& scene);
	void CreateController(physx::PxPhysics& physics, physx::PxScene& scene, physx::PxVec3 _pos, std::string& name);
public:

	physx::PxRigidDynamic* aActor = nullptr;
	physx::PxRigidStatic* aStaticActor = nullptr;
	physx::PxShape* aShape;

	physx::PxReal mass;
	physx::PxReal density;
	physx::PxTransform trans;

	physx::PxQuat physics_rot;
	physx::PxVec3 physics_scale;


	PhysicsShapeEnum physicsShapeEnum;
	int selectedShape;
	bool bCreatePhysicsComp;

	int triangleMeshStride;
	int convexMeshDetail;


	bool isCharacter;
	bool hasLineOfSight;
private:

	physx::PxMaterial* aMaterial;
	physx::PxVec3 current_scale;
	std::vector<Vertex>* verts;
	std::vector<DWORD>* indices;

	bool hasInit = false;
};

