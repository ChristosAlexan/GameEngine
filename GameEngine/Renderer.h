#pragma once
#include"DX11.h"
#include "Light.h"
#include "Entity.h"
#include "Shadows.h"
#include "PhysicsDebugDraw.h"
#include "PhysicsHandler.h"
#include "ImGui/ImGuiFileBrowser.h"
#include "CollisionObject.h"
#include "GridClass.h"
#include "NavMeshClass.h"
#include "EnvironmentProbe.h"
#include <thread>
#include "AppTimer.h"
#include "SoundComponent.h"
#include "PostProcessClass.h"
#include "PbrClass.h"
#include "GBufferClass.h"
#include "Sky.h"
#include "InstancedShape.h"

class Renderer
{
public:
	Renderer();
	bool Initialize(HWND hwnd, Camera& camera, int width, int height,std::vector<std::shared_ptr<Entity>>& entities,std::vector<Light>& lights, std::vector<Light>& pointLights);
	void Render(Camera& camera, std::vector<std::shared_ptr<Entity>>& entity, PhysicsHandler& physicsHandler, std::vector<Light>& lights, std::vector<Light>& pointLights, std::vector<CollisionObject>& collisionObjects, GridClass& grid, std::vector<NavMeshClass>& navMeshes, std::vector<SoundComponent*>& sounds, Sky& sky);
	void InitScene(std::vector<std::shared_ptr<Entity>>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera,Sky& sky);

private:
	void ClearScreen();
	void RenderDeferred(std::vector<std::shared_ptr<Entity>>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera, Sky& sky);
	void UpdateBuffers(std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera);
	void RenderToEnvProbe(EnvironmentProbe& probe, Camera& camera, std::vector<std::shared_ptr<Entity>>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Sky& sky);
	void ForwardPass(std::vector<std::shared_ptr<Entity>>& entities, Camera& camera, Sky& sky);
	void SkyRender(Camera& camera, Sky& sky, float envMapStrengthMultiplier);
	void DebugDraw(Camera& camera, std::vector<SoundComponent*>& sounds, GridClass& grid, PhysicsHandler& physicsHandler, std::vector<NavMeshClass>& navMeshes, std::vector<Light>& lights);

private:
	float rgb[4];

	InstancedShape instancedShape;

	GBufferClass gBuffer;
	PbrClass pbr;
	PostProcessClass postProcess;
	InstancedShape object;
	RectShape rect,rectSmall;
	CubeShape debugCube;

	Shadows shadowsRenderer;

	GFXGui gfxGui;
	bool enablePostProccess = true;
	bool debugEnabled = false;


	PhysicsDebugDraw physicsDebugDraw;

	int vSync;
private:
	imgui_addons::ImGuiFileBrowser file_dialog;


	bool isFileOpen;

public:
	DX11 gfx11;

	EnvironmentProbe environmentProbe;
	bool bAddEntity;
	bool isAnimated;
	bool hasTexture;
	bool isDDS;

	std::string inName;
	bool runPhysics;
	bool bAddLight;
	bool bAddPointLight;
	bool bAddCollisionObject;
	bool bClear;
	bool bCreateGrid;
	bool bRenderNavMesh;
	bool bRenderNavMeshBounds;
	bool bConvertCordinates;
	bool bEnableShadows;
	bool bModelsLoaded;
	bool save;
	bool bGuiEnabled;
	bool bDrawFrustums;
	bool bEnableSimulation;
	int switchCameraMode;
	bool copyLight;
	bool copyPointLight;
	int selectedLight = -1;
	int selectedPointLight = -1;
	double shadowBias;
	float renderDistance;
	float shadowLightsDistance, deferredLightsDistance;
	float ambientStrength;

	int listbox_item_current = -1;

	bool bEntityDeleted;


	DirectX::XMFLOAT3 skyColor;

	RectShape crosshair;
private:
	int windowWidth, windowHeight;



	RenderTexture cubeTexture;

	Texture brdfText;

	float bloomBrightness;
	float bloomStrength;
	float gamma;
	float exposure;
	float envMapStrength;
	bool bRenderCollision;


	RenderTexture forwardRenderTexture;

	AppTimer timer;


	Texture defaultText[3];

	RenderTexture finalImage;


	std::vector<Light*> culledShadowLights;


	//shadows params
	int depthBias = 40;
	double slopeBias = 1;
	float clamp = 1.0f;
};

