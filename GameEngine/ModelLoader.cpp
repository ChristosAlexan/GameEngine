#include "ModelLoader.h"
#include <assimp/pbrmaterial.h>
ModelLoader::ModelLoader()
{
	texturesLoaded = false;
	loadAsync = true;
	isTransparent = false;
	bConvertCordinates = true;
	isAttached = false;
	isTextured = true;
	isDDS = true;
}

bool ModelLoader::Initialize(const std::string filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContex, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, bool isAnimated)
{

	this->directory = StringHelper::GetDirectoryFromPath(filePath);
	if (!filePath.empty())
	{
		_filePath = filePath;
	}
	else
	{
		isTransparent = true;
		_filePath = ".//Data/Objects/cube.gltf";
	}
	this->isAnimated = isAnimated;
	_curScene = NULL;
	_prevAnimIndex = -1;
	_blendingTime = 0.0f;
	_blendingTimeMul = 1.0f;
	_updateBoth = true;
	_temporary = false;
	_playTime = 0.0f;

	this->device = device;
	this->deviceContext = deviceContex;
	this->cb_vs_vertexshader = cb_vs_vertexshader;
	deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader.GetBuffer().GetAddressOf());

	//OutputDebugStringA(("isDDS = "+ std::to_string(isDDS) + "\n").c_str());
	if (!_filePath.empty())
	{
		_asyncLoad = std::async(std::launch::async, &ModelLoader::LoadModel, this, _filePath);
		/*
		if (loadAsync)
		{
			_asyncLoad = std::async(std::launch::async, &ModelLoader::LoadModel, this, _filePath);
		}
		else
		{
			if (!LoadModel(_filePath))
			{
				return false;
			}

			if (isTextured)
			{
				for (int i = 0; i < meshes.size(); ++i)
				{
					for (int j = 0; j < meshes[i].textures.size(); ++j)
					{
						if (isDDS)
							meshes[i].textures[j].CreateTextureDDSFromWIC(device, deviceContex, meshes[i].textures[j].texturePath);
						else
							meshes[i].textures[j].CreateTextureWIC(device, meshes[i].textures[j].path);
					}
				}
			}
			
		}*/

		
	}
	return true;
}

bool ModelLoader::LoadModel(const std::string filePath)
{
	
	_curScene = importers.size();
	importers.push_back(new Assimp::Importer);

	if (importers.empty())
	{
		ErrorLogger::Log(importers[_curScene]->GetErrorString());
		return false;
	}

	scenes.push_back(importers[_curScene]->ReadFile(filePath, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes | aiProcess_FixInfacingNormals | aiProcess_ImproveCacheLocality | aiProcess_FindInvalidData | aiProcess_RemoveRedundantMaterials | aiProcess_GenSmoothNormals));

	if (!scenes[_curScene] || scenes[_curScene]->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scenes[_curScene]->mRootNode)
	{
		ErrorLogger::Log(importers[_curScene]->GetErrorString());
		return false;
	}
	aiMatrix4x4 mTransform = scenes[_curScene]->mRootNode->mTransformation;
	mTransform.Inverse();

	m_GlobalInverseTransform = DirectX::XMMatrixTranspose(DirectX::XMMATRIX(&mTransform.a1));

	this->ProcessNode(scenes[_curScene]->mRootNode, scenes[_curScene], DirectX::XMMatrixIdentity());
	
	texturesLoaded = true;

	if (isAnimated)
	{
		for (int i = 0; i < animFiles.size(); ++i)
		{
			LoadAnimation(animFiles[i]);
		}

		//OutputDebugStringA(("size" + std::to_string(scenes.size()) + "\n").c_str());
		//node_anims.resize(scenes.size());

	
	}

	node_anims.resize(node_anims.size() + 1);
	boneRotations.resize(boneRotations.size() + 1);

	bAnimLoaded = true;
	return true;
}

void ModelLoader::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContex)
{
	if (loadAsync)
	{
		if (isTextured && !bTexturesReady)
		{
			if (_asyncLoad._Is_ready())
			{
				for (int i = 0; i < meshes.size(); ++i)
				{
					for (int j = 0; j < meshes[i].textures.size(); ++j)
					{
						if (isDDS)
							meshes[i].textures[j].CreateTextureDDSFromWIC(device, deviceContex, meshes[i].textures[j].texturePath);
						else
							meshes[i].textures[j].CreateTextureWIC(device, meshes[i].textures[j].path);
					}
				}
				bTexturesReady = true;
			}
		}
	}
}

bool ModelLoader::LoadAnimation(const std::string& filePath)
{
	_curScene = importers.size();
	importers.push_back(new Assimp::Importer);
	scenes.push_back(importers[_curScene]->ReadFile(filePath, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes | aiProcess_FixInfacingNormals | aiProcess_ImproveCacheLocality | aiProcess_FindInvalidData | aiProcess_RemoveRedundantMaterials));

	if (!scenes[_curScene] || scenes[_curScene]->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scenes[_curScene]->mRootNode)
	{
		ErrorLogger::Log(importers[_curScene]->GetErrorString());
		//ErrorLogger::Log(_filePath);
		ErrorLogger::Log(filePath);
		return false;
	}

	node_anims.resize(node_anims.size() + 1);
	boneRotations.resize(boneRotations.size() + 1);

	return true;
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, const DirectX::XMMATRIX& parentTransformMatrix)
{
	DirectX::XMMATRIX nodeTransformMatrix = DirectX::XMMATRIX(&node->mTransformation.a1) * parentTransformMatrix;
	
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		meshes.push_back(this->ProcessMesh(mesh, scene, nodeTransformMatrix));
	}
	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}
}

Mesh ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const DirectX::XMMATRIX& transformMatrix)
{
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	std::vector<VertexBoneData> bones;
	bones.resize(mesh->mNumVertices);

	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		
		Vertex vertex;
		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		if (mesh->HasNormals())
		{
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}
		else
		{
			vertex.normal.x = mesh->mVertices[i].x;
			vertex.normal.y = mesh->mVertices[i].y;
			vertex.normal.z = mesh->mVertices[i].z;
		}
		if (mesh->HasTangentsAndBitangents())
		{
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;

			vertex.binormal.x = mesh->mBitangents[i].x;
			vertex.binormal.y = mesh->mBitangents[i].y;
			vertex.binormal.z = mesh->mBitangents[i].z;
		}
		else
		{
			// Generate tangents
			DirectX::XMVECTOR normalVec = XMLoadFloat3(&vertex.normal);

			DirectX::XMFLOAT3 tempFloat3 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.1f);
			DirectX::XMVECTOR tempVec = XMLoadFloat3(&tempFloat3);
			DirectX::XMVECTOR t1 = DirectX::XMVector3Cross(normalVec, tempVec);

			tempFloat3 = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			tempVec = XMLoadFloat3(&tempFloat3);
			DirectX::XMVECTOR t2 = DirectX::XMVector3Cross(normalVec, tempVec);

			DirectX::XMVECTOR t1LengthVec = DirectX::XMVector3Length(t1);
			DirectX::XMFLOAT3 t1LengthFloat3;
			XMStoreFloat3(&t1LengthFloat3, t1LengthVec);

			DirectX::XMVECTOR t2LengthVec = DirectX::XMVector3Length(t2);
			DirectX::XMFLOAT3 t2LengthFloat3;
			XMStoreFloat3(&t2LengthFloat3, t2LengthVec);

			if (t1LengthFloat3.x > t2LengthFloat3.x && t1LengthFloat3.y > t2LengthFloat3.y && t1LengthFloat3.z > t2LengthFloat3.z)
			{
				XMStoreFloat3(&vertex.tangent, t1);
			}
			else
			{
				XMStoreFloat3(&vertex.tangent, t2);
			}

			// generate binormals(bitangents)
			DirectX::XMVECTOR tangent = XMLoadFloat3(&vertex.tangent);
			DirectX::XMVector3Normalize(tangent);
			DirectX::XMVECTOR binormal = XMLoadFloat3(&vertex.binormal);
			DirectX::XMVECTOR normal = XMLoadFloat3(&vertex.normal);

			binormal = DirectX::XMVector3Cross(tangent, normal);

			XMStoreFloat3(&vertex.binormal, binormal);
		}

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);

		m_vertices.push_back(vertex);
	}

	//Get indices
	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
			m_indices.push_back(face.mIndices[j]);
		}
	}

	
	std::vector<Texture> textures;
	
	if (!texturesLoaded)
	{
		if (isTextured)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseTextures = FindMaterials(material, aiTextureType::aiTextureType_DIFFUSE, scene);
			textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
	
	
			std::vector<Texture> normalTextures = FindMaterials(material, aiTextureType::aiTextureType_NORMALS, scene);
			textures.insert(textures.end(), normalTextures.begin(), normalTextures.end());
	
			std::vector<Texture> roughnessMetallicTextures = FindMaterials(material, aiTextureType::aiTextureType_UNKNOWN, scene);
			textures.insert(textures.end(), roughnessMetallicTextures.begin(), roughnessMetallicTextures.end());
		}
	}
	


	if (isAnimated)
	{
		bonesLoaded = LoadBones(mesh, bones, vertices);

		return Mesh(this->device, this->deviceContext, vertices, indices, textures, bones, transformMatrix);
	}

	return Mesh(this->device, this->deviceContext, vertices, indices, textures, transformMatrix);
}

TextureStorageType ModelLoader::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();

	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}

	return TextureStorageType::None;
}

std::vector<Texture> ModelLoader::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0) //If there are no textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);

			switch (storetype)
			{
			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();
				
				Texture diskTexture(this->device.Get(),this->deviceContext.Get(), filename, textureType);
				materialTextures.push_back(diskTexture);
				break;
			}
			}
		}
	}
	return materialTextures;
}

std::vector<Texture> ModelLoader::FindMaterials(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0) //If there are no textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);

			switch (storetype)
			{
			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();

				Texture diskTexture(this->device.Get(), this->deviceContext.Get(), filename, textureType);
				materialTextures.push_back(diskTexture);
				break;
			}
			}
		}
	}
	return materialTextures;
}

void ModelLoader::Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, Texture* text)
{
	if (isAnimated)
	{
		DirectX::XMFLOAT4X4 _transforms;
		std::vector<DirectX::XMMATRIX> transforms;


		BoneTransform(transforms);

		XMStoreFloat4x4(&_transforms, transforms[0]);
		
	
		for (unsigned int i = 0; i < transforms.size(); ++i)
		{
			this->cb_vs_vertexshader.data.bones_transform[i] = transforms[i];
		}
	}

	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader.GetBuffer().GetAddressOf());

	this->cb_vs_vertexshader.data.viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
	this->cb_vs_vertexshader.data.projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

	for (int i = 0; i < meshes.size(); ++i)
	{
		if (!bConvertCordinates)
		{
			this->cb_vs_vertexshader.data.worldMatrix = meshes[i].GetTranformMatrix() * DirectX::XMMatrixTranspose(worldMatrix);
		}	
		else
		{
			this->cb_vs_vertexshader.data.worldMatrix = DirectX::XMMatrixTranspose(meshes[i].GetTranformMatrix() * worldMatrix);

		}
			
		

		this->cb_vs_vertexshader.UpdateBuffer();

		meshes[i].Draw(text);

	}
}


void ModelLoader::Clear()
{
	importers.clear();
	scenes.clear();
	m_vertices.clear();
	m_indices.clear();
}
