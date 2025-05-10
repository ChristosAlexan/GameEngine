#pragma once
#include "Entity.h"
#include "Camera.h"
#include "Light.h"
#include "DX11.h"
#include "GBufferClass.h"

#include <memory>
#include <future>

class Shadows
{
public:
	Shadows();
	void Initialize(DX11& gfx11, int width, int height, float aspectRatio);
	void RenderToTexture(DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entities, Camera& camera, Light* light, float& renderDistance);
	void RenderShadowEntities(ID3D11DeviceContext* ctx, DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entity, Light* light, Camera& camera, float& renderDistance);
	void RenderShadows(DX11& gfx11, std::vector<std::shared_ptr<Entity>>& entities, Light* light, Camera& camera, float& renderDistance, int& index);
	void SoftShadows(DX11& gfx11, GBufferClass& gbuffer, RectShape& rect, Camera& camera, std::vector<std::shared_ptr<Light>>& culledShadowLights);

	RenderTexture shadowRenderTexture;
	RenderTexture downSampleTexture, horizontalGaussianBlurTexture, verticalGaussianBlurTexture;
};

