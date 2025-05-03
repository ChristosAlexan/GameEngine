#pragma once
#include "Mesh.h"
#include "Animation.h"
#include <future>

class ModelLoader : public Animation
{
public:
	ModelLoader();

	bool Initialize(const std::string filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, bool isAnimated);
	bool LoadModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string filePath);
	void LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::unordered_map<std::string, Texture>& globalTextureStorage);
	bool LoadAnimation(const std::string& filePath);


	void Draw(ID3D11DeviceContext* deviceContext, const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, Texture* text = nullptr);

	void Clear();
private:
	void ProcessNode(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiNode* node, const aiScene* scene, const DirectX::XMMATRIX& parentTransformMatrix);
	Mesh ProcessMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMesh* mesh, const aiScene* scene, const DirectX::XMMATRIX& transformMatrix);
	TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	std::vector<Texture> LoadMaterialTextures(ID3D11Device* device, aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	std::vector<Texture> FindMaterials(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	
public:

	std::vector<Vertex> m_vertices;
	std::vector<DWORD> m_indices;

	bool isAnimated = false;
	bool loadAsync;

	bool isTransparent;
	bool bConvertCordinates;
	bool isAttached;
	bool isTextured;
	bool isDDS;

	std::vector<std::string> animFiles;



	std::vector<Mesh> meshes;

	DirectX::XMMATRIX parentWorldMatrix;

	std::future<bool> _asyncLoad;
private:
	//Microsoft::WRL::ComPtr<ID3D11Device> device;
	//Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;


	std::vector<Assimp::Importer*> importers;



	std::string directory;
	bool texturesLoaded;
	

	std::string _filePath;
	bool bTexturesReady = false;
};

