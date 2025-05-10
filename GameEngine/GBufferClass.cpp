#include "GBufferClass.h"
#include <PxPhysicsAPI.h>
#include "ConstantBuffersGlobals.h"
#include "ShadersGlobals.h"

GBufferClass::GBufferClass()
{
}

void GBufferClass::Initialize(DX11& gfx11, int width, int height)
{
	albedoTexture.Initialize(gfx11.device.Get(), width,height, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
	normalTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	metallicRoughnessTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	worldPositionTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	depthTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
	specularEmmissionTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	lightPassTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);

	m_renderTargetTextureArray[0] = albedoTexture.m_renderTargetTexture.Get();
	m_renderTargetTextureArray[1] = normalTexture.m_renderTargetTexture.Get();
	m_renderTargetTextureArray[2] = metallicRoughnessTexture.m_renderTargetTexture.Get();
	m_renderTargetTextureArray[3] = worldPositionTexture.m_renderTargetTexture.Get();
	m_renderTargetTextureArray[4] = depthTexture.m_renderTargetTexture.Get();
	m_renderTargetTextureArray[5] = specularEmmissionTexture.m_renderTargetTexture.Get();
	
	m_renderTargetViewArray[0] = albedoTexture.m_renderTargetView.Get();
	m_renderTargetViewArray[1] = normalTexture.m_renderTargetView.Get();
	m_renderTargetViewArray[2] = metallicRoughnessTexture.m_renderTargetView.Get();
	m_renderTargetViewArray[3] = worldPositionTexture.m_renderTargetView.Get();
	m_renderTargetViewArray[4] = depthTexture.m_renderTargetView.Get();
	m_renderTargetViewArray[5] = specularEmmissionTexture.m_renderTargetView.Get();

	m_shaderResourceViewArray[0] = albedoTexture.shaderResourceView;
	m_shaderResourceViewArray[1] = normalTexture.shaderResourceView;
	m_shaderResourceViewArray[2] = metallicRoughnessTexture.shaderResourceView;
	m_shaderResourceViewArray[3] = worldPositionTexture.shaderResourceView;
	m_shaderResourceViewArray[4] = depthTexture.shaderResourceView;
	m_shaderResourceViewArray[5] = specularEmmissionTexture.shaderResourceView;
}

void GBufferClass::GeometryPass(DX11& gfx11, Camera& camera, ID3D11DepthStencilView* depthView, float* rgb)
{
	gfx11.deviceContext->RSSetViewports(1, &gfx11.viewport);
	SetRenderTargets(gfx11, depthView);
	ClearRenderTargets(gfx11, depthView, rgb);
}

void GBufferClass::LightPass(DX11& gfx11, RectShape& rect, Camera& camera, std::vector<std::shared_ptr<Light>>& culledShadowLights, std::vector<std::shared_ptr<Light>>& pointLights, float& acceptedDist, RenderTexture& sunShadowTexture)
{
	lightPassTexture.SetRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get());
	lightPassTexture.ClearRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get(), 0, 0, 0, 1,true);


	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilDeferredPassState.Get(), 0);
	
	gfx11.deviceContext->RSSetState(gfx11.rasterizerStateFront.Get());
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_deferred.GetAddressOf());
	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		gfx11.deviceContext->PSSetShaderResources(i, 1, &m_shaderResourceViewArray[i]);
	}

	gfx11.deviceContext->OMSetBlendState(gfx11.deferredLightBlendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::deferredLightPassPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredLightPassVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredLightPassVS.GetShader(), nullptr, 0);
	for (int i = 0; i < pointLights.size(); ++i)
	{
		DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - pointLights[i]->pos.x, camera.GetPositionFloat3().y - pointLights[i]->pos.y, camera.GetPositionFloat3().z - pointLights[i]->pos.z);
		physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
		float dist = diffVec.dot(diffVec);
	
		if (dist < acceptedDist)
		{
			GFX_GLOBALS::cb_ps_pointLightsShader.data.dynamicLightColor = DirectX::XMFLOAT4(pointLights[i]->lightColor.x, pointLights[i]->lightColor.y, pointLights[i]->lightColor.z, 1.0f);
			GFX_GLOBALS::cb_ps_pointLightsShader.data.dynamicLightPosition = DirectX::XMFLOAT4(pointLights[i]->pos.x, pointLights[i]->pos.y, pointLights[i]->pos.z, 1.0f);

			GFX_GLOBALS::cb_ps_pointLightCull.data.RadiusAndcutOff.x = pointLights[i]->radius;
			GFX_GLOBALS::cb_ps_pointLightCull.data.RadiusAndcutOff.y = pointLights[i]->cutOff;
			GFX_GLOBALS::cb_ps_pointLightCull.data.RadiusAndcutOff.z = 0.0f;
			GFX_GLOBALS::cb_ps_pointLightCull.data.RadiusAndcutOff.w = 0.0f;

			GFX_GLOBALS::cb_ps_pointLightsShader.UpdateBuffer();
			GFX_GLOBALS::cb_ps_pointLightCull.UpdateBuffer();

			GFX_GLOBALS::cb_ps_materialBuffer.data.emissiveColor = pointLights[i]->emissionColor;
			GFX_GLOBALS::cb_ps_materialBuffer.data.bEmissive = 1.0f;
			GFX_GLOBALS::cb_ps_materialBuffer.UpdateBuffer();
	
	
			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - pointLights[i]->pos.x, camera.GetPositionFloat3().y - pointLights[i]->pos.y, camera.GetPositionFloat3().z - pointLights[i]->pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);
			pointLights[i]->DrawVolume(camera);
		}
	}

	std::vector< ID3D11ShaderResourceView*> ShadowTextures;

	if (!culledShadowLights.empty())
	{
		ShadowTextures.resize(culledShadowLights.size());
		int index = 0;
		for (int j = 0; j < ShadowTextures.size(); ++j)
		{

			ShadowTextures[index] = culledShadowLights[j]->m_shadowMap.shaderResourceView;
			index++;
		}
		gfx11.deviceContext->PSSetShaderResources(9, 1, &sunShadowTexture.shaderResourceView);
		gfx11.deviceContext->PSSetShaderResources(10, ShadowTextures.size(), ShadowTextures.data());
		
	}

	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::pbrPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::pbrVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::pbrVS.GetShader(), nullptr, 0);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	

	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);

	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
}

void GBufferClass::SetRenderTargets(DX11& gfx11, ID3D11DepthStencilView* depthView)
{
	gfx11.deviceContext->OMSetRenderTargets(BUFFER_COUNT, &m_renderTargetViewArray[0], depthView);
}

void GBufferClass::ClearRenderTargets(DX11& gfx11, ID3D11DepthStencilView* depthView, float* rgb)
{
	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		gfx11.deviceContext->ClearRenderTargetView(m_renderTargetViewArray[i], rgb);
	}
	gfx11.deviceContext->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
