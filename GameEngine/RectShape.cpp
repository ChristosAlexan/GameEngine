#include "RectShape.h"

RectShape::RectShape()
{
	scale = DirectX::XMFLOAT3(1.f, 1.0f, 1.0f);
}

RectShape::~RectShape()
{
}

void RectShape::Initialize(ID3D11Device* device, float aspectRatio)
{
	Vertex v[] =
	{
	Vertex(-1.0f * aspectRatio , -1.0f, 1.0f, 0.0f, 1.0f), // Bottom-left
	Vertex(-1.0f * aspectRatio ,  1.0f, 1.0f, 0.0f, 0.0f), // Top-left
	Vertex(1.0f * aspectRatio ,  1.0f, 1.0f, 1.0f, 0.0f), // Top-right
	Vertex(1.0f * aspectRatio , -1.0f, 1.0f, 1.0f, 1.0f)  // Bottom-right
	};

	std::vector<DWORD> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	HRESULT hr = this->vertexBuffer.Initialize(device, v, ARRAYSIZE(v));
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

void RectShape::CreateTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& filePath)
{

	__super::CreateTexture(device, deviceContext, filePath);
}

void RectShape::SetRenderTexture(ID3D11DeviceContext* deviceContext, RenderTexture& renderTexture)
{
	__super::SetRenderTexture(deviceContext, renderTexture);
}

void RectShape::Draw(ID3D11DeviceContext* deviceContext, Camera& camera, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	__super::Draw(deviceContext, camera, cb_vs_vertexshader);
}

void RectShape::Draw(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projectionMatrix, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	__super::Draw(deviceContext, viewMatrix, projectionMatrix, cb_vs_vertexshader);
}

void RectShape::DrawGUI(std::string name)
{
	__super::DrawGUI(name);
}
