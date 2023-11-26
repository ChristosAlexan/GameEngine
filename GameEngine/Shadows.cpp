#include "Shadows.h"

Shadows::Shadows()
{
}

void Shadows::RenderToTexture(DX11& gfx11, std::vector<Entity>& entities, Camera& camera, Light* light, float& renderDistance)
{
	light->m_shadowMap.SetRenderTarget(gfx11.deviceContext.Get(), light->m_shadowMap.m_depthStencilView);
	light->m_shadowMap.ClearRenderTarget(gfx11.deviceContext.Get(), light->m_shadowMap.m_depthStencilView, 0.0f, 0.0f, 0.0f, 1.0f);

	RenderShadowEntities(gfx11, entities, light, camera, renderDistance);
}

void Shadows::RenderShadowEntities(DX11& gfx11, std::vector<Entity>& entities, Light* light, Camera& camera,float& renderDistance)
{
	DirectX::XMMATRIX viewMatrix = (light->lightViewMatrix);
	DirectX::XMMATRIX projectionMatrix = (light->lightProjectionMatrix);
	gfx11.deviceContext->PSSetShader(gfx11.depthPS.GetShader(), NULL, 0);

	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i].model.isAttached)
		{
			if (entities[i].parent)
			{
				if (!entities[i].parentName.empty() && (entities[i].parent->entityName == entities[i].parentName))
					entities[i].SetupAttachment(entities[i].parent);
			}
		}

		if (light->lightType == 2.0f)
		{
			if (entities[i].model.isAnimated)
			{
				gfx11.deviceContext->IASetInputLayout(gfx11.depthAnimVS.GetInputLayout());
				gfx11.deviceContext->VSSetShader(gfx11.depthAnimVS.GetShader(), nullptr, 0);
			}
			else
			{
				gfx11.deviceContext->IASetInputLayout(gfx11.depthVS.GetInputLayout());
				gfx11.deviceContext->VSSetShader(gfx11.depthVS.GetShader(), nullptr, 0);
			}

			entities[i].Draw(camera, viewMatrix, projectionMatrix, light->frustumScreenDepth, nullptr, true);
		}
		else
		{
			DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(camera.GetPositionFloat3().x - entities[i].pos.x, camera.GetPositionFloat3().y - entities[i].pos.y, camera.GetPositionFloat3().z - entities[i].pos.z);
			physx::PxVec3 diffVec = physx::PxVec3(diff.x, diff.y, diff.z);
			float dist = diffVec.dot(diffVec);

			if (dist < renderDistance)
			{
				if (entities[i].model.isAnimated)
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.depthAnimVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.depthAnimVS.GetShader(), nullptr, 0);
				}
				else
				{
					gfx11.deviceContext->IASetInputLayout(gfx11.depthVS.GetInputLayout());
					gfx11.deviceContext->VSSetShader(gfx11.depthVS.GetShader(), nullptr, 0);
				}

				entities[i].Draw(camera, viewMatrix, projectionMatrix,100.0f, nullptr, true);
			}
		}
		
	}
}

void Shadows::RenderShadows(DX11& gfx11, std::vector<Entity>& entities, Light* light, Camera& camera, float& renderDistance, int& index)
{
	if (light)
	{
		light->UpdateCamera();
		gfx11.cb_vs_lightsShader.data.lightViewMatrix[index] = DirectX::XMMatrixTranspose(light->lightViewMatrix);
		gfx11.cb_vs_lightsShader.data.lightProjectionMatrix[index] = DirectX::XMMatrixTranspose(light->lightProjectionMatrix);


		gfx11.cb_vs_lightsShader.UpdateBuffer();

		gfx11.deviceContext->RSSetViewports(1, &light->m_shadowMap.m_viewport);
		RenderToTexture(gfx11, entities, camera, light, renderDistance);
	}
}
