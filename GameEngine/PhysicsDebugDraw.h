#pragma once
#include "DX11Includes.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include "Vertex.h"
#include "Mesh.h"
#include <PhysX/PxPhysicsAPI.h>
#include <memory>

class PhysicsDebugDraw
{
public:
	PhysicsDebugDraw();
	void DebugDraw(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader, const physx::PxDebugLine& line, Camera& camera);

	std::vector<Vertex> vertices;
	std::unique_ptr<Mesh> mesh;
	//Mesh* mesh;
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
};

