#include "Renderer.h"
#include <fstream>
#include <stdlib.h>
#include <ScreenGrab.h>
#include <DirectXHelpers.h>
#include <wincodec.h>
#include <random>


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
	bGuiEnabled = true;
	bDrawFrustums = false;
	switchCameraMode = 0;
	vSync = 0;
	gamma = 2.2f;
	exposure = 0.4f;
	renderDistance = 6000.0f;
	shadowLightsDistance = 2000.0f;
	deferredLightsDistance = 1000.0f;
	bloomBrightness = 3.0f;
	bloomStrength = 0.2f;
	shadowBias = 0.0000001;

	ambientStrength = 1.0f;
}

bool Renderer::Initialize(HWND hwnd, Camera& camera, int width, int height, std::vector<Entity>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights)
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
	camera.PerspectiveFov(70.0f, static_cast<float>(this->windowWidth / this->windowHeight) * (16.0f / 9.0f), 0.1f, 1000.0f);
	gfx11.deviceContext->ClearRenderTargetView(gfx11.renderTargetView.Get(), rgb);
	gfx11.deviceContext->ClearDepthStencilView(gfx11.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return true;
}

void Renderer::InitScene(std::vector<Entity>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera, Sky& sky)
{
	//camera.pos = DirectX::XMFLOAT3(1, 1, 1);
	//INIT CONSTANT BUFFERS/////////////////////////////
	///////////////////////////////////////////////////
	HRESULT hr = gfx11.cb_vs_vertexshader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_vs_lightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_vs_windowParams.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_vs_instanceShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_vs_inverseCoordsBuffer.Initialize(gfx11.device, gfx11.deviceContext);

	hr = gfx11.cb_ps_lightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	//hr = gfx11.cb_ps_PCFshader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_lightCull.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_screenEffectBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_pbrBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_materialBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_ssaoBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_pointLightCull.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_pointLightsShader.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_skyBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_shadowsBuffer.Initialize(gfx11.device, gfx11.deviceContext);
	hr = gfx11.cb_ps_inverseCoordsBuffer.Initialize(gfx11.device, gfx11.deviceContext);

	gfx11.deviceContext->VSSetConstantBuffers(0, 1, gfx11.cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(1, 1, gfx11.cb_vs_lightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(2, 1, gfx11.cb_vs_windowParams.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(3, 1, gfx11.cb_vs_instanceShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->VSSetConstantBuffers(4, 1, gfx11.cb_vs_inverseCoordsBuffer.GetBuffer().GetAddressOf());

	gfx11.deviceContext->PSSetConstantBuffers(0, 1, gfx11.cb_ps_lightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(1, 1, gfx11.cb_ps_pointLightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(2, 1, gfx11.cb_ps_ssaoBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(3, 1, gfx11.cb_ps_lightCull.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(4, 1, gfx11.cb_ps_screenEffectBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(5, 1, gfx11.cb_ps_pbrBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(6, 1, gfx11.cb_ps_materialBuffer.GetBuffer().GetAddressOf());
	//gfx11.deviceContext->PSSetConstantBuffers(6, 1, gfx11.cb_ps_pointLightsShader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(7, 1, gfx11.cb_ps_pointLightCull.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(8, 1, gfx11.cb_ps_skyBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(9, 1, gfx11.cb_ps_shadowsBuffer.GetBuffer().GetAddressOf());
	gfx11.deviceContext->PSSetConstantBuffers(10, 1, gfx11.cb_ps_inverseCoordsBuffer.GetBuffer().GetAddressOf());
	//////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	
	//gfx11.renderTexture.Initialize(gfx11.device.Get(), gfx11.windowWidth, gfx11.windowHeight);
	//finalImage.Initialize(gfx11.device.Get(), gfx11.windowWidth, gfx11.windowHeight);

	gfx11.renderTexture.Initialize(gfx11.device.Get(), windowWidth,windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	finalImage.Initialize(gfx11.device.Get(), windowWidth, windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	sky.Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), gfx11.cb_vs_vertexshader);

	AppTimer load_Timer;

	load_Timer.Start();
	for (int i = 0; i < entities.size(); ++i)
	{
		if (!entities[i].isDeleted)
		{
			entities[i].model.loadAsync = true;

			entities[i].Intitialize(entities[i].filePath, gfx11.device.Get(), gfx11.deviceContext.Get(), gfx11.cb_vs_vertexshader, entities[i].isAnimated);
		}
		if (!entities[i].isDeleted)
		{
			if (entities[i].entityName == " ")
			{
				entities[i].entityName = "Entity" + std::to_string(i);
			}
		}
	}
	
	load_Timer.Stop();
	float val = load_Timer.GetMilisecondsElapsed();
	std::string v_t_str = std::to_string(val);
	OutputDebugStringA(("time elapsed = " + v_t_str + "\n").c_str());
	rect.Initialize(gfx11.device.Get(),gfx11.windowWidth,gfx11.windowHeight);
	//postProcess.rectBloom.Initialize(gfx11.device.Get(), gfx11.windowWidth, gfx11.windowHeight);
	rectSmall.Initialize(gfx11.device.Get(), windowWidth, gfx11.windowHeight);
	debugCube.Initialize(gfx11.device.Get());


	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i].Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), gfx11.cb_vs_vertexshader);
		if (lights[i].lightType == 2.0f)
		{
			lights[i].m_shadowMap.InitializeShadow(gfx11.device.Get(), gfx11.deviceContext.Get(), 2048*4, 2048*4, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT);
			//lights[i].m_shadowMap.InitializeShadow(gfx11.device.Get(), gfx11.deviceContext.Get(), 6144,6144, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT);
		}
		else
		{
			lights[i].m_shadowMap.InitializeShadow(gfx11.device.Get(), gfx11.deviceContext.Get(), 1024,1024, DXGI_FORMAT_R16_FLOAT);
		}

		lights[i].SetupCamera(gfx11.windowWidth, gfx11.windowHeight);
	}
	for (int i = 0; i < pointLights.size(); ++i)
	{
		pointLights[i].lightType = 0;
		pointLights[i].Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), gfx11.cb_vs_vertexshader);
	}

	environmentProbe.Initialize(gfx11.device.Get(), gfx11.deviceContext.Get(), gfx11.cb_vs_vertexshader, 512, 512);
	environmentProbe.UpdateCamera();


	//Bloom
	postProcess.Initialize(gfx11, windowWidth, windowHeight);
	postProcess.HbaoPlusInit(gfx11, windowWidth, windowHeight);

	//Volumetric light
	forwardRenderTexture.Initialize(gfx11.device.Get(), windowWidth, windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

	pbr.Initialize(gfx11);
	gBuffer.Initialize(gfx11, windowWidth, windowHeight);

	defaultText[0].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-albedo.dds");
	defaultText[1].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-normal2b.dds");
	defaultText[2].CreateTextureDDS(gfx11.device.Get(),gfx11.deviceContext.Get(), ".//Data/Textures/DefaultTextures/Tex1/plasticpattern1-metalness-plasticpattern1-roughness2.dds");


	instancedShape.Initialize(gfx11.device.Get());

	crosshair.Initialize(gfx11.device.Get(), 1, 1);
	crosshair.CreateTexture(gfx11.device.Get(), gfx11.deviceContext.Get(), ".//Data/Textures/crosshair.dds");
	crosshair.pos = DirectX::XMFLOAT3(0.0, 0.0, 9.229);
	crosshair.scale.x = 0.17f;
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
void Renderer::RenderDeferred(std::vector<Entity>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera, Sky& sky)
{
	gfx11.deviceContext->VSSetConstantBuffers(0, 1, gfx11.cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());

	gfx11.deviceContext->PSSetShader(gfx11.deferredPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);


	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	for (int i = 0; i < entities.size(); ++i)
	{
		
		//bHasFinishedLoading = true; 
		if (entities[i].model.loadAsync)
		{

			if (!entities[i].model._asyncLoad._Is_ready())
			{
				//entities[i].model._asyncLoad.wait();
				bHasFinishedLoading = true;
			}
			else
			{
				bHasFinishedLoading = false;
			}
		}
		bHasFinishedLoading = true;
		//skyEntity.pos = camera.pos;
		
		

		DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i].pos.x, camera.GetPositionFloat3().y - entities[i].pos.y, camera.GetPositionFloat3().z - entities[i].pos.z);
		physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
		float dist = diffVec.dot(diffVec);

		if (dist < renderDistance)
		{
			if (!entities[i].model.isTransparent)
			{
				if (entities[i].isEmissive)
				{
					gfx11.cb_ps_materialBuffer.data.emissiveColor = entities[i].emissiveColor;
					gfx11.cb_ps_materialBuffer.data.bEmissive = 1.0f;
					gfx11.cb_ps_materialBuffer.UpdateBuffer();
				}
				else
				{
					gfx11.cb_ps_materialBuffer.data.emissiveColor = DirectX::XMFLOAT3(0,0,0);
					gfx11.cb_ps_materialBuffer.data.bEmissive = 0.0f;
					gfx11.cb_ps_materialBuffer.UpdateBuffer();
				}

				if (entities[i].model.isAnimated)
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.animDeferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.animDeferredVS.GetShader(), nullptr, 0);
				}
				else
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.deferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.deferredVS.GetShader(), nullptr, 0);
				}

				if (entities[i].model.isAttached)
				{
					if (entities[i].parent)
					{
						if (!entities[i].parentName.empty() && (entities[i].parent->entityName == entities[i].parentName))
							entities[i].SetupAttachment(entities[i].parent);
					}
				}

				entities[i].Draw(camera, camera.GetViewMatrix(), camera.GetProjectionMatrix(),100.0f, defaultText);
			}


		}
	}

	gfx11.deviceContext->IASetInputLayout(gfx11.deferredVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.deferredVS.GetShader(), nullptr, 0);
	for (int i = 0; i < lights.size(); ++i)
	{
		if (lights[i].lightType != 2.0f)
		{
			gfx11.cb_ps_materialBuffer.data.emissiveColor = lights[i].emissionColor;
			gfx11.cb_ps_materialBuffer.data.bEmissive = 1.0f;
			gfx11.cb_ps_materialBuffer.UpdateBuffer();

			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - lights[i].pos.x, camera.GetPositionFloat3().y - lights[i].pos.y, camera.GetPositionFloat3().z - lights[i].pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				lights[i].Draw(camera);
			}
		}
	}
	
	for (int i = 0; i < pointLights.size(); ++i)
	{
		gfx11.cb_ps_materialBuffer.data.emissiveColor = pointLights[i].emissionColor;
		gfx11.cb_ps_materialBuffer.data.bEmissive = 1.0f;
		gfx11.cb_ps_materialBuffer.UpdateBuffer();
	
		pointLights[i].Draw(camera);
	}

	gfx11.cb_ps_materialBuffer.data.emissiveColor = DirectX::XMFLOAT3(1.0,10.0,1.0);
	gfx11.cb_ps_materialBuffer.data.bEmissive = 1.0f;
	gfx11.cb_ps_materialBuffer.UpdateBuffer();

	gfx11.deviceContext->IASetInputLayout(gfx11.instancedVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.instancedVS.GetShader(), nullptr, 0);
	//instancedShape.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader,gfx11.cb_vs_instanceShader);

	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->PSSetShader(gfx11.testPS.GetShader(), nullptr, 0);
}
//*************************************************************************

void Renderer::UpdateBuffers(std::vector<Light>& lights, std::vector<Light>& pointLights, Camera& camera)
{
	gfx11.cb_vs_windowParams.data.window_width = (float)windowWidth;
	gfx11.cb_vs_windowParams.data.window_height = (float)windowHeight;
	culledShadowLights.clear();
	//std::vector<Light*> culledShadowLights;
	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i].UpdateCamera();

		if (lights[i].bShadow)
		{
			if (culledShadowLights.size() < NO_LIGHTS - 1)
			{
				if (lights[i].lightType == 2.0f)
				{
					culledShadowLights.push_back(&lights[i]);
				}
				else
				{
					DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - lights[i].pos.x, camera.GetPositionFloat3().y - lights[i].pos.y, camera.GetPositionFloat3().z - lights[i].pos.z);
					physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
					float dist = diffVec.dot(diffVec);

					if (lights[i].m_shadowMap.m_depthStencilView)
					{
						if (dist < shadowLightsDistance)
						{
							culledShadowLights.push_back(&lights[i]);
						}
					}
				}
			}
		}
		
	}
	for (int i = 0; i < culledShadowLights.size(); ++i)
	{
		gfx11.cb_vs_lightsShader.data.lightProjectionMatrix[i] = culledShadowLights[i]->GetCamera()->GetProjectionMatrix();
		gfx11.cb_vs_lightsShader.data.lightViewMatrix[i] = culledShadowLights[i]->GetCamera()->GetViewMatrix();
	
		gfx11.cb_ps_lightsShader.data.dynamicLightColor[i] = DirectX::XMFLOAT4(culledShadowLights[i]->lightColor.x, culledShadowLights[i]->lightColor.y, culledShadowLights[i]->lightColor.z, culledShadowLights[i]->lightColor.w);
	
		if (culledShadowLights[i]->lightType == 2.0f)
		{
			culledShadowLights[i]->pos.x = camera.pos.x;
			culledShadowLights[i]->pos.z = camera.pos.z;
		}
			
		gfx11.cb_ps_lightsShader.data.lightProjectionMatrix[i] = culledShadowLights[i]->GetCamera()->GetProjectionMatrix();
		gfx11.cb_ps_lightsShader.data.lightViewMatrix[i] = culledShadowLights[i]->GetCamera()->GetViewMatrix();

		gfx11.cb_ps_lightsShader.data.dynamicLightPosition[i] = DirectX::XMFLOAT4(culledShadowLights[i]->pos.x, culledShadowLights[i]->pos.y, culledShadowLights[i]->pos.z, 1.0f);
		
		if(culledShadowLights[i]->lightType == 2.0f)
			gfx11.cb_ps_lightsShader.data.SpotlightDir[i] = DirectX::XMFLOAT4(culledShadowLights[i]->direction.x, culledShadowLights[i]->direction.y, culledShadowLights[i]->direction.z, 1.0f);
		else
			gfx11.cb_ps_lightsShader.data.SpotlightDir[i] = DirectX::XMFLOAT4(culledShadowLights[i]->SpotDir.x, culledShadowLights[i]->SpotDir.y, culledShadowLights[i]->SpotDir.z, 1.0f);
	
		gfx11.cb_ps_lightsShader.data.lightType[i].x = culledShadowLights[i]->lightType;
	
		gfx11.cb_ps_lightsShader.data.lightType[i].y = culledShadowLights[i]->bShadow;
		gfx11.cb_ps_lightsShader.data.lightType[i].z = 0;
		gfx11.cb_ps_lightsShader.data.lightType[i].w = 0;
	
		gfx11.cb_ps_lightCull.data.RadiusAndcutOff[i].x = culledShadowLights[i]->radius;
		gfx11.cb_ps_lightCull.data.RadiusAndcutOff[i].y = culledShadowLights[i]->cutOff;
		gfx11.cb_ps_lightCull.data.RadiusAndcutOff[i].z = 0.0;
		gfx11.cb_ps_lightCull.data.RadiusAndcutOff[i].w = 0.0;



		gfx11.cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].x = culledShadowLights[i]->shadowsSoftnessBias.x;
		gfx11.cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].y = culledShadowLights[i]->shadowsSoftnessBias.y;
		gfx11.cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].z = 0.0f;
		gfx11.cb_ps_shadowsBuffer.data.shadowsSoftnessBias[i].w = 0.0f;
	}
	gfx11.cb_ps_shadowsBuffer.data.bias = shadowBias;

	if (!culledShadowLights.empty())
	{
		gfx11.cb_ps_lightsShader.data.lightsSize = culledShadowLights.size();
		gfx11.cb_vs_lightsShader.data.lightsSize = culledShadowLights.size();
	}
	else
	{
		gfx11.cb_ps_lightsShader.data.lightsSize = 0;
		gfx11.cb_vs_lightsShader.data.lightsSize = 0;
	}
	
	gfx11.cb_ps_lightsShader.data.cameraPos.x = camera.pos.x;
	gfx11.cb_ps_lightsShader.data.cameraPos.y = camera.pos.y;
	gfx11.cb_ps_lightsShader.data.cameraPos.z = camera.pos.z;
	gfx11.cb_ps_lightsShader.data.cameraPos.w = 1.0f;


	gfx11.cb_ps_screenEffectBuffer.data.gamma = gamma;
	gfx11.cb_ps_screenEffectBuffer.data.bloomBrightness = bloomBrightness;
	gfx11.cb_ps_screenEffectBuffer.data.bloomStrength = bloomStrength;
	gfx11.cb_ps_screenEffectBuffer.data.ambientStrength = ambientStrength;
	gfx11.cb_ps_screenEffectBuffer.data.exposure = exposure;
	gfx11.cb_ps_screenEffectBuffer.data.banding1 = 0;
	gfx11.cb_ps_screenEffectBuffer.data.banding2 = 0;
	gfx11.cb_ps_screenEffectBuffer.data.banding3 = 0;

	gfx11.cb_ps_inverseCoordsBuffer.data.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
	gfx11.cb_ps_inverseCoordsBuffer.data.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());

	gfx11.cb_ps_inverseCoordsBuffer.data.invProjectionMatrix = DirectX::XMMatrixInverse(nullptr, gfx11.cb_ps_inverseCoordsBuffer.data.projectionMatrix);
	gfx11.cb_ps_inverseCoordsBuffer.data.invViewMatrix = DirectX::XMMatrixInverse(nullptr, gfx11.cb_ps_inverseCoordsBuffer.data.viewMatrix);

	gfx11.cb_ps_inverseCoordsBuffer.data.cameraPos.x = camera.pos.x;
	gfx11.cb_ps_inverseCoordsBuffer.data.cameraPos.y = camera.pos.y;
	gfx11.cb_ps_inverseCoordsBuffer.data.cameraPos.z = camera.pos.z;
	gfx11.cb_ps_inverseCoordsBuffer.data.cameraPos.w = 1.0f;



	gfx11.cb_vs_inverseCoordsBuffer.data.invProjectionMatrix = DirectX::XMMatrixInverse(nullptr, gfx11.cb_ps_inverseCoordsBuffer.data.projectionMatrix);
	gfx11.cb_vs_inverseCoordsBuffer.data.invViewMatrix = DirectX::XMMatrixInverse(nullptr, gfx11.cb_ps_inverseCoordsBuffer.data.viewMatrix);

	gfx11.cb_vs_vertexshader.UpdateBuffer();
	gfx11.cb_vs_lightsShader.UpdateBuffer();
	gfx11.cb_vs_windowParams.UpdateBuffer();
	gfx11.cb_vs_instanceShader.UpdateBuffer();
	gfx11.cb_vs_inverseCoordsBuffer.UpdateBuffer();

	gfx11.cb_ps_lightsShader.UpdateBuffer();
	gfx11.cb_ps_ssaoBuffer.UpdateBuffer();
	gfx11.cb_ps_lightCull.UpdateBuffer();
	gfx11.cb_ps_screenEffectBuffer.UpdateBuffer();
	gfx11.cb_ps_pbrBuffer.UpdateBuffer();
	gfx11.cb_ps_materialBuffer.UpdateBuffer();
	gfx11.cb_ps_pointLightsShader.UpdateBuffer();
	gfx11.cb_ps_pointLightCull.UpdateBuffer();
	gfx11.cb_ps_skyBuffer.UpdateBuffer();
	gfx11.cb_ps_shadowsBuffer.UpdateBuffer();

	gfx11.cb_ps_inverseCoordsBuffer.UpdateBuffer();
}

//********************************PBR*********************************

//**********************************************************************************


void Renderer::Render(Camera& camera, std::vector<Entity>& entities, PhysicsHandler& physicsHandler, std::vector<Light>& lights, std::vector<Light>& pointLights, std::vector< CollisionObject>& collisionObjects, GridClass& grid, std::vector<NavMeshClass>& navMeshes, std::vector<SoundComponent*>& sounds, Sky& sky)
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
	shadowRasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
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
					shadowsRenderer.RenderShadows(gfx11, entities, culledShadowLights[i], camera, shadowLightsDistance, i);
					gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
					//ClearScreen();

					//shadowsRenderer.SoftShadows(gfx11, postProcess.rectBloom, culledShadowLights[i], camera);

				}

			}
		}

	}
	////////////
	environmentProbe.UpdateCamera();

	UpdateBuffers(lights, pointLights, camera);


	
	if (bHasFinishedLoading)
	{
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
	}

	gBuffer.GeometryPass(gfx11, camera, gfx11.depthStencilView.Get(), rgb);
	RenderDeferred(entities, lights, pointLights, camera,sky);
	ClearScreen();
	gfx11.deviceContext->VSSetConstantBuffers(0, 1, gfx11.cb_vs_vertexshader.GetBuffer().GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	ClearScreen();

	gfx11.deviceContext->PSSetShaderResources(5, 1, &pbr.prefilterCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(6, 1, &pbr.brdfTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(7, 1, &pbr.irradianceCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(8, 1, &postProcess.SsrRenderTexture.shaderResourceView);

	gBuffer.LightPass(gfx11, rect, camera,culledShadowLights,pointLights,deferredLightsDistance);

	gfx11.deviceContext->OMSetBlendState(nullptr, NULL, 0xFFFFFFFF);

	//////////////BLOOM///////////////////////////////////////
	if (enablePostProccess)
	{
		postProcess.HbaoPlusRender(gfx11, rect, camera, gBuffer.m_shaderResourceViewArray[4], gBuffer.m_shaderResourceViewArray[5]);
		postProcess.BloomRender(gfx11, rect, camera);

		
		//gfx11.deviceContext->PSSetShaderResources(5, 1, &gfx11.renderTexture.shaderResourceView);
		//postProcess.SSR_Render(gfx11, rect, camera, gBuffer);

		
	}
	ClearScreen();


	////////////////////////////////

	//////////////////////////////////////////////////////
	
	if (bGuiEnabled)
	{

		gfx11.deviceContext->RSSetViewports(1, &finalImage.m_viewport);
		finalImage.SetRenderTarget(gfx11.deviceContext.Get(), finalImage.m_depthStencilView);
		finalImage.ClearRenderTarget(gfx11.deviceContext.Get(), finalImage.m_depthStencilView, 0, 0, 0, 1.0f);
	}
	else
	{
		ClearScreen();
	}

	gfx11.deviceContext->PSSetShader(gfx11.postProccessPS.GetShader(), nullptr, 0);
	rect.SetRenderTexture(gfx11.deviceContext.Get(), gfx11.renderTexture);
	gfx11.deviceContext->PSSetShaderResources(1, 1, &postProcess.BloomHorizontalBlurTexture.shaderResourceView);
	//gfx11.deviceContext->PSSetShaderResources(2, 1, &forwardRenderTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(3, 1, &postProcess.hbaoTexture.shaderResourceView);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(gfx11.vs2D.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.vs2D.GetShader(), nullptr, 0);
	rect.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	//////////////////////////////////////////////////////////////////////////////////

	SkyRender(camera, sky);
	ForwardPass(entities, camera, sky);
	

	/////////////////////////////////////////////////////////////////////////////////

	DebugDraw(camera, sounds, grid, physicsHandler, navMeshes, lights);
	
	
	gfx11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	/////////////////////
	////////////////////
	
	if(bGuiEnabled)
	{
		ClearScreen();
	}
	//GUI
	////////////////////////////////////
	////////////////////////////////////
	
	gfxGui.BeginRender();

	if (bGuiEnabled)
	{
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
			ImGui::InputFloat3("Camera", &camera.pos.x);
			ImGui::Checkbox("Show collision", &physicsHandler.bRenderCollisionShape);
			ImGui::Checkbox("DrawFrustums", &bDrawFrustums);
			ImGui::Checkbox("runPhysics", &runPhysics);
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
			ImGui::InputInt("cameraMode", &switchCameraMode);
			ImGui::DragFloat("renderDistance", &renderDistance, 1.0f);
			ImGui::DragFloat("renderShadowDistance", &shadowLightsDistance, 1.0f);
			ImGui::DragFloat("deferredLightsDistance", &deferredLightsDistance, 1.0f);

			rect.DrawGUI("Rect");
		}
		
		if (ImGui::CollapsingHeader("HBAO+"))
		{
			ImGui::DragFloat("Radius", &postProcess.radius, 0.05f, 0.0f, 100.0f);
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

				lights[i].name = "light" + std::to_string(i);
				lightNames.push_back(lights[i].name.c_str());
			}
			ImGui::ListBox("Lights", &listbox_light_current, lightNames.data(), lightNames.size());
			for (int i = 0; i < lights.size(); ++i)
			{
				if (lightNames[listbox_light_current] == lights[i].name.c_str())
				{
					selectedLight = i;
					lights[i].DrawGui("light");
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

				pointLights[i].name = "pointLight" + std::to_string(i);
				lightNames.push_back(pointLights[i].name.c_str());
			}
			ImGui::ListBox("pointLights", &listbox_light_current, lightNames.data(), lightNames.size());
			for (int i = 0; i < pointLights.size(); ++i)
			{
				if (lightNames[listbox_light_current] == pointLights[i].name.c_str())
				{
					selectedPointLight = i;
					pointLights[i].DrawGui("pointLight");
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
			objNames.push_back(entities[i].entityName.c_str());
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
				if (!entities[i].isDeleted)
				{
					if (entities[i].entityName == objNames[listbox_item_current])
					{
						if (entities[i].physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i].physicsComponent.aActor->getShapes(&_shape, entities[i].physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
		
		
						}
						else if (entities[i].physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i].physicsComponent.aStaticActor->getShapes(&_shape, entities[i].physicsComponent.aStaticActor->getNbShapes());
							if(_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
						}
						entities[i].DrawGui(*physicsHandler.aScene, entities);
					}
					else
					{
						if (entities[i].physicsComponent.aActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i].physicsComponent.aActor->getShapes(&_shape, entities[i].physicsComponent.aActor->getNbShapes());
							if (_shape)
								_shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
						}
						else if (entities[i].physicsComponent.aStaticActor)
						{
							physx::PxShape* _shape = nullptr;
							entities[i].physicsComponent.aStaticActor->getShapes(&_shape, entities[i].physicsComponent.aStaticActor->getNbShapes());
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
				if (!entities[i].isDeleted)
				{
					if (entities[i].isSelected)
					{
						for (int j = 0; j < objNames.size(); ++j)
						{
							if (objNames[j] == entities[i].entityName)
							{
								listbox_item_current = j;
								entities[i].DrawGui(*physicsHandler.aScene, entities);
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
			ImGui::Checkbox("DebugLineOfSight", &physicsHandler.bDebugLineOfSight);
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


void Renderer::RenderToEnvProbe(EnvironmentProbe& probe,Camera& camera, std::vector<Entity>& entities, std::vector<Light>& lights, std::vector<Light>& pointLights, Sky& sky)
{
	//UpdateBuffers(lights,pointLights, camera);
	environmentProbe.UpdateCamera();
	//float rgb[4] = { 0.0f,0.0f,0.0f,1.0f };
	unsigned int _width = 256;
	unsigned int _height = 256;
	unsigned int maxMipLevels = 1;
	unsigned int mip = 0;
	environmentProbe.environmentCubeMap.CreateCubeMap(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, DXGI_FORMAT_R16G16B16A16_FLOAT, maxMipLevels);
	environmentProbe.environmentCubeMap.CreateCubeMapMipLevels(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, mip);

	gfx11.deviceContext->RSSetViewports(1, &environmentProbe.environmentCubeMap.m_viewport);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
	gfx11.deviceContext->PSSetSamplers(1, 1, gfx11.samplerState_Clamp.GetAddressOf());
	gfx11.deviceContext->PSSetSamplers(2, 1, gfx11.samplerState_MipMap.GetAddressOf());


	///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->VSSetConstantBuffers(0, 1, gfx11.cb_vs_vertexshader.GetBuffer().GetAddressOf());

	gfx11.deviceContext->PSSetShader(gfx11.envProbePS.GetShader(), nullptr, 0);
	
	for (int face = 0; face < 6; ++face)
	{
	
		environmentProbe.environmentCubeMap.RenderCubeMapFaces(gfx11.device.Get(), gfx11.deviceContext.Get(), face, gfx11.depthStencilView.Get(), rgb,true);

		gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
		gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->PSSetShader(gfx11.skyPS.GetShader(), nullptr, 0);
		gfx11.cb_ps_materialBuffer.data.emissiveColor = sky.color;
		gfx11.cb_ps_materialBuffer.data.bEmissive = 0.0f;
		gfx11.cb_ps_materialBuffer.UpdateBuffer();
		//gfx11.deviceContext->PSSetShader(gfx11.lightPS.GetShader(), nullptr, 0);
		gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());

		gfx11.cb_ps_skyBuffer.data.apexColor = sky.apexColor;
		gfx11.cb_ps_skyBuffer.data.centerColor = sky.centerColor;

		gfx11.cb_ps_skyBuffer.UpdateBuffer();
		gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[4]);
		sky.Draw(gfx11.deviceContext.Get(), environmentProbe.camera[face], gfx11.cb_vs_vertexshader);

		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
		for (int i = 0; i < entities.size(); ++i)
		{
			//if (entities[i].physicsComponent.mass == 0.0f)
			//{
				if (entities[i].isEmissive)
				{
					gfx11.deviceContext->PSSetShader(gfx11.lightPS.GetShader(), nullptr, 0);
					gfx11.cb_ps_materialBuffer.data.emissiveColor = entities[i].emissiveColor;
					gfx11.cb_ps_materialBuffer.UpdateBuffer();
				}
				else
					gfx11.deviceContext->PSSetShader(gfx11.envProbePS.GetShader(), nullptr, 0);

				gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
				gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
				entities[i].Draw(probe.camera[face], probe.camera[face].GetViewMatrix(), probe.camera[face].GetProjectionMatrix(), 100.0f, defaultText);
			//}

		}
	}
}

void Renderer::ForwardPass(std::vector<Entity>& entities, Camera& camera, Sky& sky)
{
	//forwardRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get());
	//forwardRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get(), 0, 0, 0, 1, false);


	gfx11.deviceContext->PSSetShaderResources(5, 1, &pbr.prefilterCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(6, 1, &pbr.brdfTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(7, 1, &pbr.irradianceCubeMap.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(8, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->PSSetShader(gfx11.transparentPbrPS.GetShader(), nullptr, 0);

	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i].model.isTransparent)
		{
			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i].pos.x, camera.GetPositionFloat3().y - entities[i].pos.y, camera.GetPositionFloat3().z - entities[i].pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				if (entities[i].model.isAnimated)
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.animDeferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.animDeferredVS.GetShader(), nullptr, 0);
				}
				else
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.deferredVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.deferredVS.GetShader(), nullptr, 0);
				}
				if (entities[i].model.isAttached)
				{
					if (entities[i].parent)
					{
						if (!entities[i].parentName.empty() && (entities[i].parent->entityName == entities[i].parentName))
							entities[i].SetupAttachment(entities[i].parent);
					}
				}

				entities[i].Draw(camera, camera.GetViewMatrix(), camera.GetProjectionMatrix(), 100.0f, defaultText);
			}
		}

		if (bDrawFrustums && entities[i].isfrustumEnabled)
		{
			gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
			gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
			gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
			gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
			gfx11.deviceContext->PSSetShader(gfx11.alphaBlendPS.GetShader(), nullptr, 0);
		
			entities[i].FrustumDraw(camera, gfx11.deviceContext.Get(), camera.GetViewMatrix(), camera.GetProjectionMatrix(), gfx11.cb_vs_vertexshader,true);
		}
		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	}

	if (runPhysics)
	{
		gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
		gfx11.deviceContext->PSSetShaderResources(0, 1, crosshair.texture.GetTextureResourceViewAddress());
		gfx11.deviceContext->IASetInputLayout(gfx11.vs2D.GetInputLayout());
		gfx11.deviceContext->VSSetShader(gfx11.vs2D.GetShader(), nullptr, 0);

		//gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
		//gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->PSSetShader(gfx11.alphaBlendPS.GetShader(), nullptr, 0);

		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
		crosshair.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
	}


	gfx11.deviceContext->OMSetBlendState(nullptr, NULL, 0xFFFFFFFF);
}

void Renderer::SkyRender(Camera& camera, Sky& sky)
{
	sky.scale.x = 100;
	sky.scale.y = 100;
	sky.scale.z = 100;

	gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetShader(gfx11.skyPS.GetShader(), nullptr, 0);
	gfx11.cb_ps_materialBuffer.data.emissiveColor = sky.color;
	//gfx11.cb_ps_materialBuffer.data.bEmissive = 1.0f;


	gfx11.cb_ps_skyBuffer.data.apexColor = sky.apexColor;
	gfx11.cb_ps_skyBuffer.data.centerColor = sky.centerColor;

	gfx11.cb_ps_skyBuffer.UpdateBuffer();
	gfx11.cb_ps_materialBuffer.UpdateBuffer();

	//gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
	sky.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
}

void Renderer::DebugDraw(Camera& camera, std::vector<SoundComponent*>& sounds, GridClass& grid, PhysicsHandler& physicsHandler, std::vector<NavMeshClass>& navMeshes, std::vector<Light>& lights)
{

	gfx11.deviceContext->PSSetShader(gfx11.testPS.GetShader(), nullptr, 0);
	rectSmall.pos = DirectX::XMFLOAT3(2.88, -1.56, 2.878);
	//gfx11.deviceContext->PSSetShaderResources(0, 1, &postProcess.SsrRenderTexture.shaderResourceView);
	//gfx11.deviceContext->PSSetShaderResources(0, 1, &postProcess.hbaoTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &lights[0].m_shadowMap.shaderResourceView);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(gfx11.vs2D.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.vs2D.GetShader(), nullptr, 0);

	if (debugEnabled)
		rectSmall.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);

	gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
	debugCube.SetRenderTexture(gfx11.deviceContext.Get(), environmentProbe.environmentCubeMap);
	//debugCube.SetRenderTexture(gfx11.deviceContext.Get(), pbr.irradianceCubeMap);
	//gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[4]);
	//debugCube.SetRenderTexture(gfx11.deviceContext.Get(), pbr.prefilterCubeMap);
	gfx11.deviceContext->PSSetShader(gfx11.cubeMapPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
	//debugCube.pos = DirectX::XMFLOAT3(0, 0, 0);

	if (debugEnabled)
		debugCube.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);

	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->PSSetShader(gfx11.colorPS.GetShader(), nullptr, 0);

	if (debugEnabled)
		environmentProbe.Draw(camera);
	



	if (!runPhysics)
	{
		gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
		gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
		gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->PSSetShader(gfx11.lightPS.GetShader(), nullptr, 0);
		for (int i = 0; i < sounds.size(); ++i)
		{
			sounds[i]->Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	//Physics Debug Draw
	//////////////////////
	//////////////////////
	gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[4]);
	gfx11.deviceContext->IASetInputLayout(gfx11.testVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(gfx11.testVS.GetShader(), nullptr, 0);
	gfx11.deviceContext->PSSetShader(gfx11.colorPS.GetShader(), nullptr, 0);

	physicsHandler.DrawDebugLine(gfx11, physicsDebugDraw, camera);

	gfx11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	if (bRenderNavMeshBounds)
	{
		grid.DrawBounds(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
		grid.DrawGrid(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader, grid.nodes);
	}
	if (bRenderNavMesh)
	{
		//if (grid.hasFinished)
		//{
		for (int i = 0; i < navMeshes.size(); ++i)
			grid.DrawGrid(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader, navMeshes[i].validNodes);
		//}

	}
}
