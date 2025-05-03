#include "Mesh.h"

Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, DirectX::XMMATRIX transformMatrix)
{
	//this->deviceContext = deviceContext;
	this->transformMatrix = transformMatrix;
	HRESULT hr = this->vertexBuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");
}


Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices,std::vector<Texture>& textures, DirectX::XMMATRIX transformMatrix)
{

	//this->deviceContext = deviceContext;
	this->transformMatrix = transformMatrix;
	if(!textures.empty())
		this->textures = textures;
	HRESULT hr = this->vertexBuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
	
	//std::string path = "Data/Objects/floor/curved-wet-cobble_normal-dx.png";
	//tex.CreateTextureWIC(device.Get(), path);
}


Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, std::vector<Texture>& textures, std::vector<VertexBoneData>& bones, DirectX::XMMATRIX transformMatrix)
{
	//this->deviceContext = deviceContext;
	this->transformMatrix = transformMatrix;
	if (!textures.empty())
		this->textures = textures;
	HRESULT hr = this->vertexBuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");

	hr = this->boneDataBuffer.Initialize(device, bones.data(), bones.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");
}


void Mesh::Clear()
{
	textures.clear();
}

void Mesh::Draw(ID3D11DeviceContext* deviceContext, Texture* text)
{
	ID3D11Buffer* buffers[2] = { vertexBuffer.Get(),boneDataBuffer.Get() };
	const UINT stride[2] = { vertexBuffer.Stride(),boneDataBuffer.Stride() };
	UINT offset =  0 ;

	deviceContext->IASetVertexBuffers(0, 2, buffers, stride, &offset);

	if (!textures.empty())
	{
		for (int i = 0; i < textures.size(); i++)
		{
			if (textures[i].GetTextureResourceView())
			{
				if (textures[i].GetType() == aiTextureType::aiTextureType_DIFFUSE)
				{
					deviceContext->PSSetShaderResources(0, 1, textures[i].GetTextureResourceViewAddress());
				}
				if (textures[i].GetType() == aiTextureType::aiTextureType_NORMALS)
				{
					deviceContext->PSSetShaderResources(1, 1, textures[i].GetTextureResourceViewAddress());
				}
				if (textures[i].GetType() == aiTextureType::aiTextureType_UNKNOWN)
				{
					deviceContext->PSSetShaderResources(2, 1, textures[i].GetTextureResourceViewAddress());
				}
			}
			else
			{
				if(albedoTexture)
					deviceContext->PSSetShaderResources(0, 1, albedoTexture.get());
				if(normalTexture)
					deviceContext->PSSetShaderResources(1, 1, normalTexture.get());
				if(roughMetalTexture)
					deviceContext->PSSetShaderResources(2, 1, roughMetalTexture.get());
			}
			
		}
	}
	else
	{
		if (text)
		{
			deviceContext->PSSetShaderResources(0, 1,	text[0].GetTextureResourceViewAddress());
			deviceContext->PSSetShaderResources(1, 1,	text[1].GetTextureResourceViewAddress());
			deviceContext->PSSetShaderResources(2, 1,	text[2].GetTextureResourceViewAddress());
		}
		
	}
	

	if (indexBuffer.Get())
	{
		deviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexed(this->indexBuffer.IndexCount(), 0, 0);
	}
	else
	{
		deviceContext->Draw(vertexBuffer.Stride(), 0);
	}
}

const DirectX::XMMATRIX& Mesh::GetTranformMatrix()
{
	return transformMatrix;
}
