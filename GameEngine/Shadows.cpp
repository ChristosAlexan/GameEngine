#include "Shadows.h"
#include "ConstantBuffersGlobals.h"
#include "ShadersGlobals.h"

Shadows::Shadows()
{
}

void Shadows::Initialize(DX11& gfx11, int width, int height, float aspectRatio)
{
	shadowRenderTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	downSampleTexture.Initialize(gfx11.device.Get(), width / 2, height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
	horizontalGaussianBlurTexture.Initialize(gfx11.device.Get(), width / 2, height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
	verticalGaussianBlurTexture.Initialize(gfx11.device.Get(), width / 2, height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);

}

void Shadows::RenderToTexture(DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entities, Camera& camera, Light* light, float& renderDistance)
{
	light->m_shadowMap.SetRenderTarget(gfx11.deviceContext.Get(), light->m_shadowMap.m_depthStencilView.Get());
	light->m_shadowMap.ClearRenderTarget(gfx11.deviceContext.Get(), light->m_shadowMap.m_depthStencilView.Get(), 0.0f, 0.0f, 0.0f, 1.0f);
	
	RenderShadowEntities(gfx11.deviceContext.Get(), gfx11, entities, light, camera, renderDistance);
}

void Shadows::RenderShadowEntities(ID3D11DeviceContext* ctx, DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entities, Light* light, Camera& camera,float& renderDistance)
{
	DirectX::XMMATRIX viewMatrix = (light->lightViewMatrix);
	DirectX::XMMATRIX projectionMatrix = (light->lightProjectionMatrix);
	ctx->PSSetShader(GFX_GLOBALS::depthPS.GetShader(), NULL, 0);

	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i]->model.isTransparent)
			continue;
		if (entities[i]->model.isAttached)
		{
			if (entities[i]->parent)
			{
				if (!entities[i]->parentName.empty() && (entities[i]->parent->entityName == entities[i]->parentName))
					entities[i]->SetupAttachment(entities[i]->parent);
			}
		}

		if (light->lightType == 2.0f)
		{
			if (entities[i]->model.isAnimated)
			{
				ctx->IASetInputLayout(GFX_GLOBALS::depthAnimVS.GetInputLayout());
				ctx->VSSetShader(GFX_GLOBALS::depthAnimVS.GetShader(), nullptr, 0);
			}
			else
			{
				ctx->IASetInputLayout(GFX_GLOBALS::depthVS.GetInputLayout());
				ctx->VSSetShader(GFX_GLOBALS::depthVS.GetShader(), nullptr, 0);
			}

			entities[i]->Draw(ctx, camera, viewMatrix, projectionMatrix, light->frustumScreenDepth, nullptr, true);
		}
		else
		{
			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i]->pos.x, camera.GetPositionFloat3().y - entities[i]->pos.y, camera.GetPositionFloat3().z - entities[i]->pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				if (entities[i]->model.isAnimated)
				{
					ctx->IASetInputLayout(GFX_GLOBALS::depthAnimVS.GetInputLayout());
					ctx->VSSetShader(GFX_GLOBALS::depthAnimVS.GetShader(), nullptr, 0);
				}
				else
				{
					ctx->IASetInputLayout(GFX_GLOBALS::depthVS.GetInputLayout());
					ctx->VSSetShader(GFX_GLOBALS::depthVS.GetShader(), nullptr, 0);
				}

				entities[i]->Draw(ctx, camera, viewMatrix, projectionMatrix,100.0f, nullptr, true);
			}
		}
		
	}
}

void Shadows::RenderShadows(DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entities, Light* light, Camera& camera, float& renderDistance, int& index)
{
	if (light)
	{
		light->UpdateCamera();
		GFX_GLOBALS::cb_vs_lightsShader.data.lightViewMatrix[index] = DirectX::XMMatrixTranspose(light->lightViewMatrix);
		GFX_GLOBALS::cb_vs_lightsShader.data.lightProjectionMatrix[index] = DirectX::XMMatrixTranspose(light->lightProjectionMatrix);


		GFX_GLOBALS::cb_vs_lightsShader.UpdateBuffer();

		gfx11.deviceContext->RSSetViewports(1, &light->m_shadowMap.m_viewport);
		RenderToTexture(gfx11, entities, camera, light, renderDistance);
	}
}

void Shadows::SoftShadows(DX11& gfx11, GBufferClass& gbuffer, RectShape& rect, Camera& camera, std::vector<std::shared_ptr<Light>>& culledShadowLights)
{
	gfx11.deviceContext->RSSetViewports(1, &shadowRenderTexture.m_viewport);
	shadowRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get());
	shadowRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), gfx11.depthStencilView.Get(), 0, 0, 0, 1, true);

	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		gfx11.deviceContext->PSSetShaderResources(i, 1, &gbuffer.m_shaderResourceViewArray[i]);
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
		gfx11.deviceContext->PSSetShaderResources(9, ShadowTextures.size(), ShadowTextures.data());
	}

	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::shadowPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::pbrVS.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::pbrVS.GetShader(), nullptr, 0);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);



	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);

	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());


	//Guassian blur downSample
	gfx11.deviceContext->RSSetViewports(1, &downSampleTexture.m_viewport);
	downSampleTexture.SetRenderTarget(gfx11.deviceContext.Get(), downSampleTexture.m_depthStencilView.Get());
	downSampleTexture.ClearRenderTarget(gfx11.deviceContext.Get(), downSampleTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);
	
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::downSampleBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &shadowRenderTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
	
	//Guassian blur horizontal
	gfx11.deviceContext->RSSetViewports(1, &horizontalGaussianBlurTexture.m_viewport);
	horizontalGaussianBlurTexture.SetRenderTarget(gfx11.deviceContext.Get(), horizontalGaussianBlurTexture.m_depthStencilView.Get());
	horizontalGaussianBlurTexture.ClearRenderTarget(gfx11.deviceContext.Get(), horizontalGaussianBlurTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);
	
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::shadowHorizontalGaussianBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &downSampleTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
	
	//Guassian blur vertical
	gfx11.deviceContext->RSSetViewports(1, &verticalGaussianBlurTexture.m_viewport);
	verticalGaussianBlurTexture.SetRenderTarget(gfx11.deviceContext.Get(), verticalGaussianBlurTexture.m_depthStencilView.Get());
	verticalGaussianBlurTexture.ClearRenderTarget(gfx11.deviceContext.Get(), verticalGaussianBlurTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);
	
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::shadowVerticalGaussianBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &horizontalGaussianBlurTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
}