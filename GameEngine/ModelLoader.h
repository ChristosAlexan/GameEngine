#pragma once
#include "Mesh.h"
#include "Animation.h"
#include <future>

class ModelLoader : public Animation
{
public:
	ModelLoader();

	bool Initialize(const std::string filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContex, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, bool isAnimated);
	bool LoadModel(const std::string filePath);
	void LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContex, std::unordered_map<std::string, Texture>& globalTextureStorage);
	bool LoadAnimation(const std::string& filePath);


	void Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, Texture* text = nullptr);

	void Clear();
private:
	void ProcessNode(aiNode* node, const aiScene* scene, const DirectX::XMMATRIX& parentTransformMatrix);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const DirectX::XMMATRIX& transformMatrix);
	TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	std::vector<Texture> FindMaterials(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	
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
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;


	std::vector<Assimp::Importer*> importers;



	std::string directory;
	bool texturesLoaded;
	

	std::string _filePath;
	bool bTexturesReady = false;
};

