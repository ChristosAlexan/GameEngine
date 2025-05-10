#include "Renderer.h"
#include <fstream>
#include <stdlib.h>
#include <ScreenGrab.h>
#include <DirectXHelpers.h>
#include <wincodec.h>
#include <random>
#include "ConstantBuffersGlobals.h"
#include "ShadersGlobals.h"

Renderer::Renderer()
{
	timer.Start();

	skyColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	bEntityDeleted = false;
	save = false;
	hasTexture = true;
	isDDS = true;
	isAnimated = false;
	isFileOpen = false;
	bAddEntity = false;
	runPhysics = false;
	bAddLight = false;
	bAddPointLight = false;
	bAddCollisionObject = false;
	bClear = false;
	bCreateGrid = false;
	bRenderNavMesh = false;
	bRenderNavMeshBounds = false;
	bConvertCordinates = false;
	bModelsLoaded = false;
	bRenderCollision = false;
	copyLight = false;
	copyPointLight = false;
	bEnableShadows = true;
	enablePostProccess = true;
	bGuiEnabled = true;
	bDrawFrustums = false;
	bEnableSimulation = false;
	switchCameraMode = 0;
	vSync = 0;
	gamma = 2.2f;
	exposure = 0.4f;
	envMapStrength = 10.0f;
	renderDistance = 6000.0f;
	shadowLightsDistance = 2000.0f;
	deferredLightsDistance = 1000.0f;
	bloomBrightness = 0.3f;
	bloomStrength = 0.25f;
	shadowBias = 0.001;
	aspectRatio = 0.0f;
	ambientStrength = 0.2f;

	depthBias = 1000;
	slopeBias = 1.5f;
	clamp = 0.0f;

	GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.x = 0;
	GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.y = 0;
	GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.z = 0;
	GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.w = 0;
}

bool Renderer::Initialize(HWND hwnd, Camera& camera, int width, int height, std::vector<std::shared_ptr<Entity>>& entities, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights)
{
	this->windowWidth = width;
	this->windowHeight = height;

	if (!gfx11.Initialize(hwnd, camera, windowWidth, windowHeight))
		return false;

	gfxGui.Initialize(hwnd, gfx11.device.Get(), gfx11.deviceContext.Get());



	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);

	gfx11.deviceContext->RSSetViewports(1, &gfx11.viewport);
	gfx11.deviceContext->OMSetRenderTargets(1, gfx11.renderTargetView.GetAddressOf(), gfx11.depthStencilView.Get());
	aspectRatio = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
	camera.PerspectiveFov(90.0f, aspectRatio, 0.1f, 100.0f);
	gfx11.deviceContext->ClearRenderTargetView(gfx11.renderTargetView.Get(), rgb);
	gfx11.deviceContext->ClearDepthStencilView(gfx11.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return true;
}

void Renderer::InitScene(std::vector<std::shared_ptr<Entity>>& entities, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, Camera& camera, Sky& sky)
{
	//camera.pos = DirectX::XMFLOAT3(1, 1, 1);
	//INIT CONSTANT BUFFERS/////////////////////////////
	///////////////////////////////////////////////////
	HRESULT hr = GFX_GLOBALS::cb_vs_vertexshader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_vs_ssrBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_vs_lightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_vs_windowParams.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_vs_instanceShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_vs_inverseCoordsBuffer.Initialize(gfx11.device, gfx11.deviceContext);

	hr = GFX_GLOBALS::cb_ps_lightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	//hr = GFX_GLOBALS::cb_ps_PCFshader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_lightCull.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_screenEffectBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_pbrBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_materialBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_cameraBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_pointLightCull.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_pointLightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_skyBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_shadowsBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_inverseCoordsBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = GFX_GLOBALS::cb_ps_ssaoBuffer.Initialize(gfx11.device, gfx11.deviceContext);

	gfx11.deviceContext->VSSetConstantBuffers(0, 1, GFX_GLOBALS::cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(1, 1, GFX_GLOBALS::cb_vs_lightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(2, 1, GFX_GLOBALS::cb_vs_windowParams.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(3, 1, GFX_GLOBALS::cb_vs_instanceShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(4, 1, GFX_GLOBALS::cb_vs_inverseCoordsBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(5, 1, GFX_GLOBALS::cb_vs_ssrBuffer.GetBuffer().GetAddressOf());

	gfx11.deviceContext->PSSetConstantBuffers(0, 1, GFX_GLOBALS::cb_ps_lightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(1, 1, GFX_GLOBALS::cb_ps_pointLightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(2, 1, GFX_GLOBALS::cb_ps_cameraBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(3, 1, GFX_GLOBALS::cb_ps_lightCull.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(4, 1, GFX_GLOBALS::cb_ps_screenEffectBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(5, 1, GFX_GLOBALS::cb_ps_pbrBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(6, 1, GFX_GLOBALS::cb_ps_materialBuffer.GetBuffer().GetAddressOf());
	//gfx11.deviceContext->PSSetConstantBuffers(6, 1, GFX_GLOBALS::cb_ps_pointLightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(7, 1, GFX_GLOBALS::cb_ps_pointLightCull.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(8, 1, GFX_GLOBALS::cb_ps_skyBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(9, 1, GFX_GLOBALS::cb_ps_shadowsBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(10, 1, GFX_GLOBALS::cb_ps_inverseCoordsBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(11, 1, GFX_GLOBALS::cb_ps_ssaoBuffer.GetBuffer().GetAddressOf());
	//////////////////////////////////////////////////////
	/////////////////////////////////////////////////////

	gfx11.renderTexture.Initialize(gfx11.device.Get(), windowWidth,windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	finalImage.Initialize(gfx11.device.Get(), windowWidth, windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	sky.Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), GFX_GLOBALS::cb_vs_vertexshader);

	AppTimer load_Timer;

	load_Timer.Start();
	for (int i = 0; i < entities.size(); ++i)
	{
		if (!entities[i]->isDeleted)
		{
			entities[i]->model.loadAsync = true;
			entities[i]->Intitialize(entities[i]->filePath, gfx11.device.Get(), gfx11.deviceContext.Get(), GFX_GLOBALS::cb_vs_vertexshader, entities[i]->isAnimated);
		}
		if (!entities[i]->isDeleted)
		{
			if (entities[i]->entityName == " ")
			{
				entities[i]->entityName = "Entity" + std::to_string(i);
			}
		}
	}
	
	load_Timer.Stop();
	float val = load_Timer.GetMilisecondsElapsed();
	std::string v_t_str = std::to_string(val);
	OutputDebugStringA(("time elapsed = " + v_t_str + "\n").c_str());
	rect.Initialize(gfx11.device.Get(), aspectRatio);
	//postProcess.rectBloom.Initialize(gfx11.device.Get(), gfx11.windowWidth, gfx11.windowHeight);
	rectSmall.Initialize(gfx11.device.Get(), aspectRatio);
	debugCube.Initialize(gfx11.device.Get());


	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i]->Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), GFX_GLOBALS::cb_vs_vertexshader);
		if (lights[i]->lightType == 2.0f)
		{
			lights[i]->m_shadowMap.InitializeShadow(gfx11.device.Get(), gfx11.deviceContext.Get(), 2048, 2048, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT);
		}
		else
		{
			lights[i]->m_shadowMap.InitializeShadow(gfx11.device.Get(), gfx11.deviceContext.Get(), 1024,1024, DXGI_FORMAT_R16_FLOAT);
		}

		lights[i]->SetupCamera(gfx11.windowWidth, gfx11.windowHeight);
	}
	for (int i = 0; i < pointLights.size(); ++i)
	{
		pointLights[i]->lightType = 0;
		pointLights[i]->Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), GFX_GLOBALS::cb_vs_vertexshader);
	}
	shadowsRenderer.Initialize(gfx11, gfx11.windowWidth, gfx11.windowHeight, aspectRatio);

	environmentProbe.Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), GFX_GLOBALS::cb_vs_vertexshader, 512, 512);
	environmentProbe.UpdateCamera();


	//Bloom
	postProcess.Initialize(gfx11, windowWidth, windowHeight, aspectRatio);
	postProcess.HbaoPlusInit(gfx11, windowWidth, windowHeight);
	forwardRenderTexture.Initialize(gfx11.device.Get(), windowWidth, windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	deferredForwardCombinedTexture.Initialize(gfx11.device.Get(), windowWidth, windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

	pbr.Initialize(gfx11);
	gBuffer.Initialize(gfx11, windowWidth, windowHeight);

	defaultText[0].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-albedo.dds");
	defaultText[1].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-normal2b.dds");
	defaultText[2].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-metalness-plasticpattern1-roughness2.dds");


	instancedShape.Initialize(gfx11.device.Get());

	crosshair.Initialize(gfx11.device.Get(), aspectRatio);
	crosshair.CreateTexture(gfx11.device.Get(), gfx11.deviceContext.Get(), ".//Data/Textures/crosshair.dds");
	crosshair.pos = DirectX::XMFLOAT3(0.0, 0.0, 9.229);
	crosshair.scale.x = 0.1f;
	crosshair.scale.y = 0.1f;
	//crosshair.texture.CreateTextureWIC(gfx11.device.Get(), ".//Data/Textures/crosshair.png");
}

void Renderer::ClearScreen()
{
	gfx11.deviceContext->RSSetViewports(1, &gfx11.viewport);
	gfx11.deviceContext->OMSetRenderTargets(1, gfx11.renderTargetView.GetAddressOf(), gfx11.depthStencilView.Get());
	gfx11.deviceContext->ClearRenderTargetView(gfx11.renderTargetView.Get(), rgb);
	gfx11.deviceContext->ClearDepthStencilView(gfx11.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

//****************RENDER ENTITIES DEFERRED***************************************
void Renderer::RenderDeferred(std::vector<std::shared_ptr<Entity>>& entities, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, Camera& camera, Sky& sky)
{
	gfx11.deviceContext->VSSetConstantBuffers(0, 1, GFX_GLOBALS::cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());

	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::deferredPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);


	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	for (int i = 0; i < entities.size(); ++i)
	{
		GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = entities[i]->emissiveColor;
		GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 0.0f;
		GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
		DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i]->pos.x, camera.GetPositionFloat3().y - entities[i]->pos.y, camera.GetPositionFloat3().z - entities[i]->pos.z);
		physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
		float dist = diffVec.dot(diffVec);

		if (dist < renderDistance)
		{
			if (!entities[i]->model.isTransparent)
			{
				if (entities[i]->isEmissive)
				{
					GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = entities[i]->emissiveColor;
					GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 1.0f;
					GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
				}
				else
				{
					GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = DirectX::XMFLOAT3(0,0,0);
					GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 0.0f;
					GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
				}

				if (entities[i]->model.isAnimated)
				{
					gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::animDeferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(GFX_GLOBALS::animDeferredVS.GetShader(), nullptr, 0);
				}
				else
				{
					gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
				}

				if (entities[i]->model.isAttached)
				{
					if (entities[i]->parent)
					{
						if (!entities[i]->parentName.empty() && (entities[i]->parent->entityName == entities[i]->parentName))
							entities[i]->SetupAttachment(entities[i]->parent);
					}
				}

				entities[i]->Draw(gfx11.deviceContext.Get(), camera, camera.GetViewMatrix(), camera.GetProjectionMatrix(), 100.0f, defaultText);
			}


		}
	}

	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
	for (int i = 0; i < lights.size(); ++i)
	{
		if (lights[i]->lightType != 2.0f)
		{
			GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = lights[i]->emissionColor;
			GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 1.0f;
			GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();

			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - lights[i]->pos.x, camera.GetPositionFloat3().y - lights[i]->pos.y, camera.GetPositionFloat3().z - lights[i]->pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				lights[i]->Draw(gfx11.deviceContext.Get(), camera);
			}
		}
	}
	
	for (int i = 0; i < pointLights.size(); ++i)
	{
		GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = pointLights[i]->emissionColor;
		GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 1.0f;
		GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
	
		pointLights[i]->Draw(gfx11.deviceContext.Get(), camera);
	}


	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::instancedVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::instancedVS.GetShader(), nullptr, 0);
	//instancedShape.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader,GFX_GLOBALS::cb_vs_instanceShader);

	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::testPS.GetShader(), nullptr, 0);
}
//*************************************************************************

void Renderer::UpdateBuffers(std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, Camera& camera)
{
	GFX_GLOBALS::cb_vs_windowParams.data.window_width = (float)windowWidth;
	GFX_GLOBALS::cb_vs_windowParams.data.window_height = (float)windowHeight;
	culledShadowLights.clear();
	//std::vector<Light*> culledShadowLights;
	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i]->UpdateCamera();

		if (lights[i]->bShadow)
		{
			if (culledShadowLights.size() < NO_LIGHTS - 1)
			{
				if (lights[i]->lightType == 2.0f)
				{
					culledShadowLights.push_back(lights[i]);
				}
				else
				{
					DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - lights[i]->pos.x, camera.GetPositionFloat3().y - lights[i]->pos.y, camera.GetPositionFloat3().z - lights[i]->pos.z);
					physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
					float dist = diffVec.dot(diffVec);

					if (lights[i]->m_shadowMap.m_depthStencilView)
					{
						if (dist < shadowLightsDistance)
						{
							culledShadowLights.push_back(lights[i]);
						}
					}
				}
			}
		}
		
	}
	for (int i = 0; i < culledShadowLights.size(); ++i)
	{
		GFX_GLOBALS::cb_vs_lightsShader.data.lightProjectionMatrix[i] = DirectX::XMMatrixTranspose(culledShadowLights[i]->GetCamera()->GetProjectionMatrix());
		GFX_GLOBALS::cb_vs_lightsShader.data.lightViewMatrix[i] = DirectX::XMMatrixTranspose(culledShadowLights[i]->GetCamera()->GetViewMatrix());
	
		GFX_GLOBALS::cb_ps_lightsShader.data.dynamicLightColor[i] = DirectX::XMFLOAT4(culledShadowLights[i]->lightColor.x, culledShadowLights[i]->lightColor.y, culledShadowLights[i]->lightColor.z, culledShadowLights[i]->lightColor.w);
	
		if (culledShadowLights[i]->lightType == 2.0f)
		{
			culledShadowLights[i]->pos.x = camera.pos.x;
			culledShadowLights[i]->pos.z = camera.pos.z;
		}
			
		GFX_GLOBALS::cb_ps_lightsShader.data.lightProjectionMatrix[i] = DirectX::XMMatrixTranspose((culledShadowLights[i]->GetCamera()->GetProjectionMatrix()));
		GFX_GLOBALS::cb_ps_lightsShader.data.lightViewMatrix[i] = DirectX::XMMatrixTranspose((culledShadowLights[i]->GetCamera()->GetViewMatrix()));

		GFX_GLOBALS::cb_ps_lightsShader.data.dynamicLightPosition[i] = DirectX::XMFLOAT4(culledShadowLights[i]->pos.x, culledShadowLights[i]->pos.y, culledShadowLights[i]->pos.z, 1.0f);
		
		if(culledShadowLights[i]->lightType == 2.0f)
			GFX_GLOBALS::cb_ps_lightsShader.data.SpotlightDir[i] = DirectX::XMFLOAT4(culledShadowLights[i]->direction.x, culledShadowLights[i]->direction.y, culledShadowLights[i]->direction.z, 1.0f);
		else
			GFX_GLOBALS::cb_ps_lightsShader.data.SpotlightDir[i] = DirectX::XMFLOAT4(culledShadowLights[i]->SpotDir.x, culledShadowLights[i]->SpotDir.y, culledShadowLights[i]->SpotDir.z, 1.0f);
	
		GFX_GLOBALS::cb_ps_lightsShader.data.lightType[i].x = culledShadowLights[i]->lightType;
	
		GFX_GLOBALS::cb_ps_lightsShader.data.lightType[i].y = culledShadowLights[i]->bShadow;
		GFX_GLOBALS::cb_ps_lightsShader.data.lightType[i].z = 0;
		GFX_GLOBALS::cb_ps_lightsShader.data.lightType[i].w = 0;
	
		GFX_GLOBALS::cb_ps_lightCull.data.RadiusAndcutOff[i].x = culledShadowLights[i]->radius;
		GFX_GLOBALS::cb_ps_lightCull.data.RadiusAndcutOff[i].y = culledShadowLights[i]->cutOff;
		GFX_GLOBALS::cb_ps_lightCull.data.RadiusAndcutOff[i].z = 0.0;
		GFX_GLOBALS::cb_ps_lightCull.data.RadiusAndcutOff[i].w = 0.0;



		GFX_GLOBALS::cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].x = culledShadowLights[i]->shadowsSoftnessBias.x;
		GFX_GLOBALS::cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].y = culledShadowLights[i]->shadowsSoftnessBias.y;
		GFX_GLOBALS::cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].z = culledShadowLights[i]->shadowsSoftnessBias.z;
		GFX_GLOBALS::cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].w = 0.0f;
	}
	GFX_GLOBALS::cb_ps_shadowsBuffer.data.bias = shadowBias;

	if (!culledShadowLights.empty())
	{
		GFX_GLOBALS::cb_ps_lightsShader.data.lightsSize = culledShadowLights.size();
		GFX_GLOBALS::cb_vs_lightsShader.data.lightsSize = culledShadowLights.size();
	}
	else
	{
		GFX_GLOBALS::cb_ps_lightsShader.data.lightsSize = 0;
		GFX_GLOBALS::cb_vs_lightsShader.data.lightsSize = 0;
	}
	
	GFX_GLOBALS::cb_ps_cameraBuffer.data.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
	GFX_GLOBALS::cb_ps_cameraBuffer.data.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
	GFX_GLOBALS::cb_ps_cameraBuffer.data.viewProjectionMatrix = DirectX::XMMatrixTranspose((DirectX::XMMatrixMultiply(camera.GetViewMatrix(), camera.GetProjectionMatrix())));
	GFX_GLOBALS::cb_ps_cameraBuffer.data.inverseViewProjectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(camera.GetViewMatrix(), camera.GetProjectionMatrix())));
	GFX_GLOBALS::cb_ps_cameraBuffer.data.inverseViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, camera.GetViewMatrix()));
	GFX_GLOBALS::cb_ps_cameraBuffer.data.inverseProjectionMatrix =DirectX::XMMatrixTranspose( DirectX::XMMatrixInverse(nullptr, camera.GetProjectionMatrix()));
	GFX_GLOBALS::cb_ps_cameraBuffer.data.cameraPos = DirectX::XMFLOAT4(camera.pos.x, camera.pos.y, camera.pos.z, 1.0f);
	GFX_GLOBALS::cb_ps_cameraBuffer.data.screenSize = DirectX::XMFLOAT2((float)windowWidth, (float)windowHeight);
	GFX_GLOBALS::cb_ps_cameraBuffer.data.nearPlane = camera.m_nearZ;
	GFX_GLOBALS::cb_ps_cameraBuffer.data.farPlane = camera.m_farZ;


	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.gamma = gamma;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.bloomBrightness = bloomBrightness;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.bloomStrength = bloomStrength;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.ambientStrength = ambientStrength;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.exposure = exposure;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.envMapStrength = envMapStrength;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.banding2 = 0;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.banding3 = 0;

	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());

	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.invProjectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.projectionMatrix));
	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.invViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.viewMatrix));

	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.cameraPos.x = camera.pos.x;
	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.cameraPos.y = camera.pos.y;
	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.cameraPos.z = camera.pos.z;
	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.cameraPos.w = 1.0f;

	GFX_GLOBALS::cb_vs_inverseCoordsBuffer.data.invProjectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.projectionMatrix));
	GFX_GLOBALS::cb_vs_inverseCoordsBuffer.data.invViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GFX_GLOBALS::cb_ps_inverseCoordsBuffer.data.viewMatrix));

	GFX_GLOBALS::cb_vs_ssrBuffer.data.viewProjectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(camera.GetViewMatrix(), camera.GetProjectionMatrix()));

	GFX_GLOBALS::cb_vs_vertexshader.UpdateBuffer();
	GFX_GLOBALS::cb_vs_lightsShader.UpdateBuffer();
	GFX_GLOBALS::cb_vs_windowParams.UpdateBuffer();
	GFX_GLOBALS::cb_vs_instanceShader.UpdateBuffer();
	GFX_GLOBALS::cb_vs_inverseCoordsBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_vs_ssrBuffer.UpdateBuffer();

	GFX_GLOBALS::cb_ps_lightsShader.UpdateBuffer();
	GFX_GLOBALS::cb_ps_cameraBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_lightCull.UpdateBuffer();
	GFX_GLOBALS::cb_ps_screenEffectBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_pbrBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_pointLightsShader.UpdateBuffer();
	GFX_GLOBALS::cb_ps_pointLightCull.UpdateBuffer();
	GFX_GLOBALS::cb_ps_skyBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_shadowsBuffer.UpdateBuffer();

	GFX_GLOBALS::cb_ps_inverseCoordsBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_cameraBuffer.UpdateBuffer();

	GFX_GLOBALS::cb_ps_ssaoBuffer.UpdateBuffer();
}

//********************************PBR*********************************

//**********************************************************************************


void Renderer::Render(Camera& camera, std::vector<std::shared_ptr<Entity>>& entities, PhysicsHandler& physicsHandler, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, std::vector< CollisionObject>& collisionObjects, GridClass& grid, std::vector<NavMeshClass>& navMeshes, std::vector<SoundComponent*>& sounds, Sky& sky)
{
	//float rgb[4];
	rgb[0] = skyColor.x;
	rgb[1] = skyColor.y;
	rgb[2] = skyColor.z;
	rgb[3] = 1.0f;

	//SHADOWS////
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizerState;
	CD3D11_RASTERIZER_DESC shadowRasterizerDesc(D3D11_DEFAULT);
	
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	shadowRasterizerDesc.DepthBias = depthBias;
	shadowRasterizerDesc.DepthBiasClamp = clamp;
	shadowRasterizerDesc.SlopeScaledDepthBias = slopeBias;
	hr = gfx11.device->CreateRasterizerState(&shadowRasterizerDesc, shadowRasterizerState.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");
	
	if (bEnableShadows)
	{
		if (!culledShadowLights.empty())
		{
			for (int i = 0; i < culledShadowLights.size(); ++i)
			{
				culledShadowLights[i]->UpdateCamera();

				if (culledShadowLights[i]->bShadow)
				{
					gfx11.deviceContext->RSSetState(shadowRasterizerState.Get());
					shadowsRenderer.RenderShadows(gfx11, entities, culledShadowLights[i].get(), camera, shadowLightsDistance, i);
					gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());

				}

			}
		}

	}

	//ClearScreen();
	////////////
	environmentProbe.UpdateCamera();

	UpdateBuffers(lights, pointLights, camera);


	
	if (environmentProbe.recalculate)
	{
		
			
		ClearScreen();

		environmentProbe.prevPos = environmentProbe.pos;
		environmentProbe.UpdateCamera();

		RenderToEnvProbe(environmentProbe, camera, entities, lights, pointLights, sky);
		pbr.PbrRender(gfx11,rect,debugCube,environmentProbe,camera,rgb);
		environmentProbe.pos = environmentProbe.prevPos;
		environmentProbe.recalculate = false;

	}

	// === GEOMETRY PASS ===
// Clear depth and stencil before geometry pass
	ClearScreen();

	gBuffer.GeometryPass(gfx11, camera, gfx11.depthStencilView.Get(), rgb);
	RenderDeferred(entities, lights, pointLights, camera, sky);



	ClearScreen();

	gfx11.deviceContext->VSSetConstantBuffers(0, 1, GFX_GLOBALS::cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());

	// === SOFT SUN SHADOWS ===
	shadowsRenderer.SoftShadows(gfx11, gBuffer, rect, camera, culledShadowLights);
	ClearScreen();

	gfx11.deviceContext->PSSetShaderResources(6, 1, &pbr.prefilterCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(7, 1, &pbr.brdfTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(8, 1, &pbr.irradianceCubeMap.shaderResourceView);

	// === LIGHTING PASS ===
	gBuffer.LightPass(gfx11, rect, camera, culledShadowLights, pointLights, deferredLightsDistance, shadowsRenderer.verticalGaussianBlurTexture);

	gfx11.deviceContext->OMSetBlendState(nullptr, NULL, 0xFFFFFFFF);


	// === FORWARD PASS ===
	forwardRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get());
	forwardRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get(), 0, 0, 0, 1, false);
	SkyRender(camera, sky, 1.0f);

	// === COMBINE PASS ===
	gfx11.deviceContext->RSSetViewports(1, &deferredForwardCombinedTexture.m_viewport);
	deferredForwardCombinedTexture.SetRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get());
	deferredForwardCombinedTexture.ClearRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get(), 0, 0, 0, 1, false);


	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::deferredForwardCombinePS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.lightPassTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(1, 1, &forwardRenderTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
	// === POST PROCESS ===
	if (enablePostProccess) {
		postProcess.HbaoPlusRender(gfx11, rect, camera, gBuffer.m_shaderResourceViewArray[4], gBuffer.m_shaderResourceViewArray[5]);
		postProcess.BloomRender(gfx11, rect, camera, gBuffer, forwardRenderTexture);
		//postProcess.SSR_Render(gfx11, rect, camera, gBuffer);
	}


	// === FINAL COMPOSITE ===
	gfx11.deviceContext->RSSetViewports(1, &finalImage.m_viewport);
	finalImage.SetRenderTarget(gfx11.deviceContext.Get(), finalImage.m_depthStencilView.Get());
	finalImage.ClearRenderTarget(gfx11.deviceContext.Get(), finalImage.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::postProccessPS.GetShader(), nullptr, 0);
	rect.SetRenderTexture(gfx11.deviceContext.Get(), deferredForwardCombinedTexture);
	gfx11.deviceContext->PSSetShaderResources(1, 1, &postProcess.verticalGaussianBlurTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(2, 1, &forwardRenderTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(3, 1, &postProcess.hbaoTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(4, 1, &postProcess.SsrRenderTexture.shaderResourceView);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), nullptr, 0);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	gfx11.deviceContext->OMSetDepthStencilState(nullptr, 0);

	// === FORWARD PASS ===
	//SkyRender(camera, sky, 1.0f);
	ForwardPass(entities, camera, sky);
	DebugDraw(camera, sounds, grid, physicsHandler, navMeshes, lights);
	
	
	gfx11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	/////////////////////
	////////////////////
	
	ClearScreen();
	
	//GUI
	////////////////////////////////////
	////////////////////////////////////
	
	gfxGui.BeginRender();

	//if (bGuiEnabled)
	//{
		gfxGui.EditorStyle();


		ImGuiWindowFlags viewport_flags;
		//ImGui::PopStyleVar(2);

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		//ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(ImVec2(gfx11.windowWidth, gfx11.windowHeight));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		viewport_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		viewport_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;

		if (finalImage.shaderResourceView)
		{
			ImTextureID textId = finalImage.shaderResourceView;


			ImGui::Begin("Viewport", nullptr, viewport_flags);

			ImGui::Image(textId, ImVec2(gfx11.windowWidth, gfx11.windowHeight));
			ImGui::End();
		}


	if (bGuiEnabled)
	{
		static bool show_app_metrics = true;
		static bool show_app_console = false;
		static bool show_app_log = false;
		static bool show_app_style_editor = false;
		static bool show_lights = false;
		static bool show_objects = true;
		static bool show_particles = false;
		static bool show_general = false;
		static bool show_help = false;

		if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }
		if (show_help) 
		{ 
			ImGui::Begin("Help Window");
			ImGui::Text("F5: Save");
			ImGui::Text("F6: Stop physics simulation");
			ImGui::Text("F7: Start physics simulation");
			ImGui::Text("F8: Possess character");
			ImGui::Text("F9: Unpossess character");

			ImGui::Text("Ctrl + C: Copy selected object");
			ImGui::Text("Ctrl + V: Paste copied object");
			ImGui::End();
		}

		
		bool open = false;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Engine"))
			{
				if (ImGui::MenuItem("Save", NULL))
					save = true;

				ImGui::EndMenu();
			}
			ImGui::SameLine();
			if (ImGui::BeginMenu("Files"))
			{
				if (ImGui::MenuItem("Open", NULL))
					open = true;

				ImGui::EndMenu();
			}
			ImGui::SameLine();

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Metrics", NULL, &show_app_metrics);

				ImGui::EndMenu();
			}
			ImGui::SameLine();
			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Key bindings", NULL, &show_help);

				ImGui::EndMenu();
			}
			ImGui::SameLine();

			ImGui::NewLine();

			if (open)
				ImGui::OpenPopup("Open File");

			/* Optional third parameter. Support opening only compressed rar/zip files.
			 * Opening any other file will show error, return false and won't close the dialog.
			 */
			if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(400, 200), "*.*,.obj,.dae,.gltf,.fbx,.glb"))
			{
				std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
				std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
				//f << file_dialog.selected_path;

				std::fstream f;
				f = std::fstream(file_dialog.selected_path.c_str());
				if (f.good())
					inName = file_dialog.selected_path;

				//OutputDebugStringA(("NAME = " + inName + "\n").c_str());

				isFileOpen = true;
			}

			ImGui::EndMainMenuBar();
		}


		physicsHandler.isMouseHover = false;

		ImGui::Begin("Options");

		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
		}
		

		if (ImGui::CollapsingHeader("Options##2"))
		{
			ImGui::InputFloat("aspectRatio", &aspectRatio);
			ImGui::Checkbox("runPhysics", &runPhysics);
			ImGui::NewLine();
			ImGui::InputFloat3("Camera", &camera.pos.x);
			ImGui::Checkbox("DebugLines", &physicsHandler.bDebugLines);
			ImGui::Checkbox("Show collision", &physicsHandler.bRenderCollisionShape);
			ImGui::Checkbox("DrawFrustums", &bDrawFrustums);
			ImGui::Checkbox("Possess", &camera.PossessCharacter);
			ImGui::Checkbox("bEnableShadows", &bEnableShadows);
			ImGui::Checkbox("enablePostProccess", &enablePostProccess);
			ImGui::Checkbox("debugEnabled", &debugEnabled);
			ImGui::Checkbox("renderCollision", &bRenderCollision);
			ImGui::DragInt("vSync", &vSync);
			ImGui::DragFloat("bloomStrengh", &bloomStrength, 0.1f);
			ImGui::DragFloat("bloomBrightness", &bloomBrightness, 0.1f);
			ImGui::DragFloat("gamma", &gamma, 0.1f);
			ImGui::DragFloat("exposure", &exposure, 0.1f);
			ImGui::DragFloat("envMapStrength", &envMapStrength, 0.1f);
			ImGui::InputInt("cameraMode", &switchCameraMode);
			ImGui::DragFloat("renderDistance", &renderDistance, 1.0f);
			ImGui::DragFloat("renderShadowDistance", &shadowLightsDistance, 1.0f);
			ImGui::DragFloat("deferredLightsDistance", &deferredLightsDistance, 1.0f);

			if (ImGui::CollapsingHeader("SSR"))
			{
				ImGui::DragFloat("val1", &GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.x, 0.01f);
				ImGui::DragFloat("val2", &GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.y, 0.01f);
				ImGui::DragFloat("val3", &GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.z, 0.01f);
				ImGui::DragFloat("val4", &GFX_GLOBALS::cb_ps_cameraBuffer.data.testValues.w, 0.01f);
			}
			rect.DrawGUI("Rect");
		}
		
		if (ImGui::CollapsingHeader("HBAO+"))
		{
			ImGui::DragFloat("Radius", &postProcess.radius, 0.005f, 0.0f, 100.0f);
			ImGui::DragFloat("Bias", &postProcess.bias, 0.001f, 0, 0.5f);
			ImGui::DragFloat("Sharpness", &postProcess.sharpness, 1.0f, 0.0f, 100.0f);
			ImGui::DragFloat("powerExponent", &postProcess.powerExponent, 1.0f, 1.0f, 10.0f);
			ImGui::DragFloat("metersToViewSpaceUnits", &postProcess.metersToViewSpaceUnits, 1.0f, 1.0f, 100.0f);
			ImGui::DragFloat("smallScaleAO", &postProcess.smallScaleAO, 0.1f, 0.0f, 2.0f);
			ImGui::DragFloat("largeScaleAO", &postProcess.largeScaleAO, 0.1f, 0.0f, 2.0f);
			ImGui::DragFloat("decodeBias", &postProcess.decodeBias, 0.1f, 0.0f, 2.0f);
			ImGui::DragFloat("decodeScale", &postProcess.decodeScale, 0.1f, 0.0f, 2.0f);
		}

		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::DragInt("depthBias", &depthBias, 1);
			ImGui::InputDouble("slopeBias", &slopeBias, 0.05f);
			ImGui::DragFloat("clamp", &clamp, 0.05f);
			ImGui::InputDouble("shadowBias", &shadowBias);

		}
		ImGui::End();

		ImGui::Begin("Sky");
		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
		}
		
		sky.DrawGui("Sky");
		
		ImGui::End();

		ImGui::Begin("Lights");
		{
			ImGui::DragFloat("ambientStrength", &ambientStrength, 0.05f,0.0f,1.0f);
			if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
			{
				physicsHandler.isMouseHover = true;
			}
			
			static int listbox_light_current = 0;
			std::vector<const char*> lightNames;
			for (int i = 0; i < lights.size(); ++i)
			{

				lights[i]->name = "light" + std::to_string(i);
				lightNames.push_back(lights[i]->name.c_str());
			}
			ImGui::ListBox("Lights", &listbox_light_current, lightNames.data(), lightNames.size());
			for (int i = 0; i < lights.size(); ++i)
			{
				if (lightNames[listbox_light_current] == lights[i]->name.c_str())
				{
					selectedLight = i;
					lights[i]->DrawGui("light");
				}
			}
			if (ImGui::Button("Add"))
			{
				bAddLight = true;
			}
			if (ImGui::Button("Copy"))
			{
				copyLight = true;
			}

		}
		
		ImGui::End();

		ImGui::Begin("pointLights");
		{
			if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
			{
				physicsHandler.isMouseHover = true;
			}
			
			static int listbox_light_current = 0;
			std::vector<const char*> lightNames;
			for (int i = 0; i < pointLights.size(); ++i)
			{

				pointLights[i]->name = "pointLight" + std::to_string(i);
				lightNames.push_back(pointLights[i]->name.c_str());
			}
			ImGui::ListBox("pointLights", &listbox_light_current, lightNames.data(), lightNames.size());
			for (int i = 0; i < pointLights.size(); ++i)
			{
				if (lightNames[listbox_light_current] == pointLights[i]->name.c_str())
				{
					selectedPointLight = i;
					pointLights[i]->DrawGui("pointLight");
				}
			}
			if (ImGui::Button("Add"))
			{
				bAddPointLight = true;
			}
			if (ImGui::Button("Copy"))
			{
				copyPointLight = true;
			}

		}
	
		
		ImGui::End();


		ImGui::Begin("Entities");
		
		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
		}
		
		std::vector<const char*> objNames;
		for (int i = 0; i < entities.size(); ++i)
		{
			objNames.push_back(entities[i]->entityName.c_str());
		}

		if (listbox_item_current > objNames.size() - 1)
		{
			listbox_item_current = -1;
		}
		ImGui::ListBox("Objects", &listbox_item_current, objNames.data(), objNames.size());

		if (listbox_item_current > -1)
		{
			for (int i = 0; i < entities.size(); ++i)
			{
				if (!entities[i]->isDeleted)
				{
					if (entities[i]->entityName == objNames[listbox_item_current])
					{
						if (entities[i]->physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i]->physicsComponent.aActor->getShapes(&_shape, entities[i]->physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
		
		
						}
						else if (entities[i]->physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i]->physicsComponent.aStaticActor->getShapes(&_shape, entities[i]->physicsComponent.aStaticActor->getNbShapes());
							if(_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
						}
						entities[i]->DrawGui(*physicsHandler.aScene, entities);
					}
					else
					{
						if (entities[i]->physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i]->physicsComponent.aActor->getShapes(&_shape, entities[i]->physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
						}
						else if (entities[i]->physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i]->physicsComponent.aStaticActor->getShapes(&_shape, entities[i]->physicsComponent.aStaticActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
						}
					}
				}
				
			}
		}
		else
		{
			for (int i = 0; i < entities.size(); ++i)
			{
				if (!entities[i]->isDeleted)
				{
					if (entities[i]->isSelected)
					{
						for (int j = 0; j < objNames.size(); ++j)
						{
							if (objNames[j] == entities[i]->entityName)
							{
								listbox_item_current = j;
								entities[i]->DrawGui(*physicsHandler.aScene, entities);
							}
						}
					}
				}
				
				
			}
		}
		
		ImGui::End();

		ImGui::Begin("AI");
		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
			//OutputDebugStringA("VALID!!!!!\n");
		}
		
		if (ImGui::CollapsingHeader("AI##2"))
		{
			grid.DrawGUI();
			ImGui::Checkbox("renderNavMesh", &bRenderNavMesh);
			ImGui::Checkbox("renderNavMeshBounds", &bRenderNavMeshBounds);
			if (ImGui::Button("Create grid"))
			{
				bCreateGrid = true;
			}
		}
		ImGui::End();

		ImGui::Begin("Sounds");
		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
			//OutputDebugStringA("VALID!!!!!\n");
		}
		if (ImGui::CollapsingHeader("Sounds##2"))
		{
			for (int i = 0; i < sounds.size(); ++i)
			{
				sounds[i]->cube.DrawGUI("sound" + std::to_string(i));
			}
		}
		ImGui::End();

		ImGui::Begin("EnvironmentProbe");
		if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
		{
			physicsHandler.isMouseHover = true;
			//OutputDebugStringA("VALID!!!!!\n");
		}
		
		if (ImGui::CollapsingHeader("EnvironmentProbe##2"))
		{
			environmentProbe.DrawGui("Probe1");
		}
		ImGui::End();

		if (isFileOpen)
		{
			ImGui::Begin("Import");
			if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))
			{
				physicsHandler.isMouseHover = true;
				//OutputDebugStringA("VALID!!!!!\n");
			}
			
			if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDragging(0))
			{
				physicsHandler.isMouseHover = true;
				//OutputDebugStringA("VALID!!!!!\n");
			}
			
			ImGui::Checkbox("Textures", &hasTexture);
			if (hasTexture)
			{
				ImGui::Checkbox("DDS", &isDDS);
			}
			ImGui::Checkbox("Animated", &isAnimated);
			ImGui::Checkbox("ConvertCordinates", &bConvertCordinates);
			if (ImGui::Button("Add"))
			{
				bAddEntity = true;
				isFileOpen = false;
			}
			ImGui::End();
		}

		ImGui::Begin("2D sprites");
		crosshair.DrawGUI("Crosshair");
		ImGui::End();

		//ImGuiWindowFlags viewport_flags;
		////ImGui::PopStyleVar(2);
		//
		//const ImGuiViewport* viewport = ImGui::GetMainViewport();
		////ImGui::SetNextWindowPos(ImVec2(0, 0));
		//ImGui::SetNextWindowPos(viewport->Pos);
		//ImGui::SetNextWindowSize(ImVec2(gfx11.windowWidth, gfx11.windowHeight));
		//ImGui::SetNextWindowViewport(viewport->ID);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		//viewport_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		//viewport_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
		//
		//if (finalImage.shaderResourceView)
		//{
		//	ImTextureID textId = finalImage.shaderResourceView;
		//
		//
		//	ImGui::Begin("Viewport", nullptr, viewport_flags);
		//
		//	ImGui::Image(textId, ImVec2(gfx11.windowWidth, gfx11.windowHeight));
		//	ImGui::End();
		//}
		

	}

	gfxGui.EndRender();

		/////////////////////////////////////
		/////////////////////////////////////

	gfx11.swapchain->Present(vSync, NULL);

}


void Renderer::RenderToEnvProbe(EnvironmentProbe& probe,Camera& camera, std::vector<std::shared_ptr<Entity>>& entities, std::vector<std::shared_ptr<Light>>& lights, std::vector<std::shared_ptr<Light>>& pointLights, Sky& sky)
{
	//UpdateBuffers(lights,pointLights, camera);
	environmentProbe.UpdateCamera();
	//float rgb[4] = { 0.0f,0.0f,0.0f,1.0f };
	unsigned int _width = 256;
	unsigned int _height = 256;
	unsigned int maxMipLevels = 1;
	unsigned int mip = 0;

	environmentProbe.environmentCubeMap.CreateCubeMap(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, DXGI_FORMAT_R16G16B16A16_FLOAT, maxMipLevels);
	//environmentProbe.environmentCubeMap.CreateCubeMapMipLevels(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, mip);
	//environmentProbe.environmentCubeMap.RenderAllCubeFaces(gfx11.device.Get(), gfx11.deviceContext.Get(),gfx11.depthStencilView.Get(), rgb, true);

	gfx11.deviceContext->RSSetViewports(1, &environmentProbe.environmentCubeMap.m_viewport);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
	gfx11.deviceContext->PSSetSamplers(1, 1, gfx11.samplerState_Clamp.GetAddressOf());
	gfx11.deviceContext->PSSetSamplers(2, 1, gfx11.samplerState_MipMap.GetAddressOf());


	///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->VSSetConstantBuffers(0, 1, GFX_GLOBALS::cb_vs_vertexshader.GetBuffer().GetAddressOf());

	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::envProbePS.GetShader(), nullptr, 0);
	
	if (!culledShadowLights.empty())
	{
		std::vector< ID3D11ShaderResourceView*> ShadowTextures;
		ShadowTextures.resize(culledShadowLights.size());
		int index = 0;
		for (int j = 0; j < ShadowTextures.size(); ++j)
		{

			ShadowTextures[index] = culledShadowLights[j]->m_shadowMap.shaderResourceView;
			index++;
		}
		gfx11.deviceContext->PSSetShaderResources(9, ShadowTextures.size(), ShadowTextures.data());
	}

	for (UINT mip = 0; mip < environmentProbe.environmentCubeMap.texElementDesc.MipLevels; ++mip)
	{
		for (int face = 0; face < 6; ++face)
		{
			environmentProbe.environmentCubeMap.RenderCubeMapFace(gfx11.device.Get(), gfx11.deviceContext.Get(), face, mip, gfx11.depthStencilView.Get(), rgb, true);
			gfx11.deviceContext->GenerateMips(environmentProbe.environmentCubeMap.shaderResourceView);

			gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
			gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
			gfx11.deviceContext->PSSetShader(GFX_GLOBALS::skyPS.GetShader(), nullptr, 0);
			GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = sky.color;
			GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 0.0f;
			GFX_GLOBALS::cb_ps_screenEffectBuffer.data.envMapStrength = 3.0f;
			GFX_GLOBALS::cb_ps_screenEffectBuffer.UpdateBuffer();
			GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();

			gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());

			gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[4]);
			SkyRender(environmentProbe.camera[face], sky, 3.0f);

			gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
			for (int i = 0; i < entities.size(); ++i)
			{
				if (entities[i]->physicsComponent.mass != 0.0f)
					continue;
				if (entities[i]->isEmissive)
				{
					gfx11.deviceContext->PSSetShader(GFX_GLOBALS::lightPS.GetShader(), nullptr, 0);
					GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = entities[i]->emissiveColor;
					GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
				}
				else
					gfx11.deviceContext->PSSetShader(GFX_GLOBALS::envProbePS.GetShader(), nullptr, 0);

				gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
				gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
				entities[i]->Draw(gfx11.deviceContext.Get(), probe.camera[face], probe.camera[face].GetViewMatrix(), probe.camera[face].GetProjectionMatrix(), 100.0f, defaultText);
			}
		}
	}

}

void Renderer::ForwardPass(std::vector<std::shared_ptr<Entity>>& entities, Camera& camera, Sky& sky)
{


	gfx11.deviceContext->PSSetShaderResources(5, 1, &pbr.prefilterCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(6, 1, &pbr.brdfTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(7, 1, &pbr.irradianceCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(8, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->PSSetShaderResources(9, 1, &finalImage.shaderResourceView);
	if (!culledShadowLights.empty())
	{
		std::vector< ID3D11ShaderResourceView*> ShadowTextures;
		ShadowTextures.resize(culledShadowLights.size());
		int index = 0;
		for (int j = 0; j < ShadowTextures.size(); ++j)
		{

			ShadowTextures[index] = culledShadowLights[j]->m_shadowMap.shaderResourceView;
			index++;
		}
		gfx11.deviceContext->PSSetShaderResources(10, ShadowTextures.size(), ShadowTextures.data());
	}
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::transparentPbrPS.GetShader(), nullptr, 0);

	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i]->model.isTransparent)
		{
			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i]->pos.x, camera.GetPositionFloat3().y - entities[i]->pos.y, camera.GetPositionFloat3().z - entities[i]->pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				if (entities[i]->model.isAnimated)
				{
					gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::animDeferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(GFX_GLOBALS::animDeferredVS.GetShader(), nullptr, 0);
				}
				else
				{
					gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
				}
				if (entities[i]->model.isAttached)
				{
					if (entities[i]->parent)
					{
						if (!entities[i]->parentName.empty() && (entities[i]->parent->entityName == entities[i]->parentName))
							entities[i]->SetupAttachment(entities[i]->parent);
					}
				}

				entities[i]->Draw(gfx11.deviceContext.Get(), camera, camera.GetViewMatrix(), camera.GetProjectionMatrix(), 100.0f, defaultText);
			}
		}

		if (bDrawFrustums && entities[i]->isfrustumEnabled)
		{
			gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
			gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
			gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
			gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
			gfx11.deviceContext->PSSetShader(GFX_GLOBALS::alphaBlendPS.GetShader(), nullptr, 0);
		
			entities[i]->FrustumDraw(camera, gfx11.deviceContext.Get(), camera.GetViewMatrix(), camera.GetProjectionMatrix(), GFX_GLOBALS::cb_vs_vertexshader,true);
		}
		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	}

	if (bEnableSimulation)
	{
		gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
		gfx11.deviceContext->PSSetShaderResources(0, 1, crosshair.texture.GetTextureResourceViewAddress());
		gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
		gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), nullptr, 0);

		//gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
		//gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->PSSetShader(GFX_GLOBALS::alphaBlendPS.GetShader(), nullptr, 0);

		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
		crosshair.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
	}


	gfx11.deviceContext->OMSetBlendState(nullptr, NULL, 0xFFFFFFFF);
}

void Renderer::SkyRender(Camera& camera, Sky& sky,float envMapStrengthMultiplier)
{
	sky.scale.x = 100;
	sky.scale.y = 100;
	sky.scale.z = 100;

	gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::skyPS.GetShader(), nullptr, 0);
	GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = sky.color;
	//GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 1.0f;


	GFX_GLOBALS::cb_ps_skyBuffer.data.apexColor = sky.apexColor;
	GFX_GLOBALS::cb_ps_skyBuffer.data.centerColor = sky.centerColor;
	envMapStrength = envMapStrengthMultiplier;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.data.envMapStrength = envMapStrength;
	GFX_GLOBALS::cb_ps_screenEffectBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_skyBuffer.UpdateBuffer();
	GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();

	//gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
	sky.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
}

void Renderer::DebugDraw(Camera& camera, std::vector<SoundComponent*>& sounds, GridClass& grid, PhysicsHandler& physicsHandler, std::vector<NavMeshClass>& navMeshes, std::vector<std::shared_ptr<Light>>& lights)
{

	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::testPS.GetShader(), nullptr, 0);
	rectSmall.pos = DirectX::XMFLOAT3(2.88, -1.56, 2.878);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), nullptr, 0);

	if (debugEnabled)
		rectSmall.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
	gfx11.deviceContext->PSSetShaderResources(0, 1, &environmentProbe.environmentCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::cubeMapPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());

	if (debugEnabled)
		debugCube.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::colorPS.GetShader(), nullptr, 0);

	if (debugEnabled)
		environmentProbe.Draw(gfx11.deviceContext.Get(), camera);
	



	if (!bEnableSimulation)
	{
		gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
		gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
		gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->PSSetShader(GFX_GLOBALS::lightPS.GetShader(), nullptr, 0);
		for (int i = 0; i < sounds.size(); ++i)
		{
			sounds[i]->Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	//Physics Debug Draw
	//////////////////////
	//////////////////////
	gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::colorPS.GetShader(), nullptr, 0);

	physicsHandler.DrawDebugLine(gfx11, physicsDebugDraw, camera);

	gfx11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	if (bRenderNavMeshBounds)
	{
		grid.DrawBounds(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
		grid.DrawGrid(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader, grid.nodes);
	}
	if (bRenderNavMesh)
	{
		//if (grid.hasFinished)
		//{
		for (int i = 0; i < navMeshes.size(); ++i)
			grid.DrawGrid(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader, navMeshes[i].validNodes);
		//}

	}
}
