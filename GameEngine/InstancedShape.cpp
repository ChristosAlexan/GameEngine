#include "InstancedShape.h"
#include <random>
#include<iostream>

//float random_between_two_float(float min, float max)
//{
//	return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
//}

void InstancedShape::Initialize(ID3D11Device* device)
{
	timer.Start();

	scale = DirectX::XMFLOAT3(0.002f, 0.002f, 0.002f);
	rot = DirectX::XMFLOAT3(0, 0, 0);
	pos = DirectX::XMFLOAT3(0, 0, 0);

    m_instanceBuffer = 0;

    InstanceType* instances;
    D3D11_BUFFER_DESC vertexBufferDesc, instanceBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, instanceData;
    HRESULT result;

	//std::vector<Vertex> vertices;
	//
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f));	 // bottom-left
	//vertices.push_back(Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f));		// top-right
	//vertices.push_back(Vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f));		 // bottom-right         
	//vertices.push_back(Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f));		// top-right
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f));	 // bottom-left
	//vertices.push_back(Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f));	// top-left
	//// front face
	//vertices.push_back(Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f));	// bottom-left
	//vertices.push_back(Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f));// bottom-right
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 1.0f));// top-right
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 1.0f));// top-right
	//vertices.push_back(Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f));// top-left
	//vertices.push_back(Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f));	// bottom-left
	//	// left face
	//vertices.push_back(Vertex(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f));		// top-right
	//vertices.push_back(Vertex(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f));	// top-left
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f));		// bottom-left
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f));		// bottom-left
	//vertices.push_back(Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f));	// bottom-right
	//vertices.push_back(Vertex(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f));	// top-right
	//									// right face
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f));// top-left
	//vertices.push_back(Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f));	// bottom-right
	//vertices.push_back(Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f));	// top-right         
	//vertices.push_back(Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f));	 // bottom-right
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f));	 // top-left
	//vertices.push_back(Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f));	// bottom-left     
	//								  // bottom face
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f));	 // top-right
	//vertices.push_back(Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f));	// top-left
	//vertices.push_back(Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f));	// bottom-left
	//vertices.push_back(Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f));	// bottom-left
	//vertices.push_back(Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f));	// bottom-right
	//vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f));		// top-right
	//									// top face
	//vertices.push_back(Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f));	 // top-left
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f));	// bottom-right
	//vertices.push_back(Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f));	// top-right     
	//vertices.push_back(Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f));	// bottom-right
	//vertices.push_back(Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f));	// top-left
	//vertices.push_back(Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f));		// bottom-left    


	Vertex vertices[] =
	{
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f), // +Y (top face)
		Vertex(1.0f, 1.0f, -1.0f,0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, 1.0f,  1.0f,0.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f),

		Vertex(-1.0f, -1.0f,  1.0f,0.0f, -1.0f, 0.0f, 0.0f, 0.0f), // -Y (bottom face)
		Vertex(1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,0.0f, 1.0f),

		Vertex(1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f), // +X (right face)
		Vertex(1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),

		Vertex(-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f), // -X (left face)
		Vertex(-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),

		Vertex(-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f), // +Z (front face)
		Vertex(1.0f,  1.0f, 1.0f , 0.0f, 0.0f, 1.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, 1.0f , 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f),

		Vertex(1.0f,  1.0f, -1.0f ,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f), // -Z (back face)
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex(1.0f, -1.0f, -1.0f ,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f),
	};

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	HRESULT hr = this->vertexBuffer.Initialize(device, vertices, ARRAYSIZE(vertices));
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices, ARRAYSIZE(indices));
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");

	m_instanceCount = 4000;

	instances = new InstanceType[m_instanceCount];
	if (!instances)
	{
		exit(0);
	}


	srand(time(NULL));

	//lamda function that returns random float
	auto random_between_two_float = [](float min, float max)
	{
		return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
	};

	for (int i = 0; i < m_instanceCount; ++i)
	{
		float r1 = random_between_two_float(-12000.0f, 12000.0f);
		float r2 = random_between_two_float(0.0f, 5000.0f);
		float r3 = random_between_two_float(-12000.0f, 12000.0f);
		instances[i].position = DirectX::XMFLOAT3(r1, r2, r3);
	}

	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the instance data.
	instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.

	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if (FAILED(result))
	{
		COM_ERROR_IF_FAILED(hr, "Failed to initialize instance buffer for mesh.");
	}

	// Release the instance array now that the instance buffer has been created and loaded.
	delete[] instances;
	instances = 0;
}

void InstancedShape::CreateTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& filePath)
{
}

void InstancedShape::Draw(ID3D11DeviceContext* deviceContext, Camera& camera, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_VS_instanceShader>& cb_vs_instanceShader)
{
	for (int i = 0; i < m_instanceCount; ++i)
	{
		float val = timer.GetMilisecondsElapsed();
		if (i %2 ==0 && i%4==0 && i % 3 != 0)
			cb_vs_instanceShader.data.pos[i] = DirectX::XMFLOAT3(std::sin(val*0.001f) * 50.0f, 0, 0);
		else if (i % 3 == 0)
			cb_vs_instanceShader.data.pos[i] = DirectX::XMFLOAT3(std::cos(val * 0.001f) * 70.0f, 0, 0);
		else if (i % 2 == 0 && i % 4 != 0 && i % 3 == 0)
			cb_vs_instanceShader.data.pos[i] = DirectX::XMFLOAT3(0, 0, std::cos(val * 0.001f) * 80.0f);
		else
			cb_vs_instanceShader.data.pos[i] = DirectX::XMFLOAT3(0, 0, std::sin(val * 0.001f) * 30.0f);
	}
	cb_vs_instanceShader.UpdateBuffer();
	DirectX::XMMATRIX m_scale = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX m_rotate = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	DirectX::XMMATRIX m_translate = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(InstanceType);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = vertexBuffer.Get();
	bufferPointers[1] = m_instanceBuffer;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader.GetBuffer().GetAddressOf());
	cb_vs_vertexshader.data.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
	cb_vs_vertexshader.data.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
	cb_vs_vertexshader.data.worldMatrix = m_rotate * m_scale * m_translate;
	cb_vs_vertexshader.data.worldMatrix = DirectX::XMMatrixTranspose(cb_vs_vertexshader.data.worldMatrix);

	cb_vs_vertexshader.UpdateBuffer();

	deviceContext->DrawIndexedInstanced(indexBuffer.IndexCount(), m_instanceCount,0 , 0, 0);
	//deviceContext->DrawInstanced(vertexBuffer.VertexCount(), m_instanceCount, 0, 0);
}

int InstancedShape::GetInstanceCount()
{
    return m_instanceCount;
}
