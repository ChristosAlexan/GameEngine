#pragma once
#include"DX11.h"
#include "Entity.h"
#include "Light.h"
#include<HBAOplus/GFSDK_SSAO.h>
#include "GBufferClass.h"
#include <random>


class PostProcessClass
{
public:
	PostProcessClass();
	void Initialize(DX11& gfx11, int width, int height, float aspectRatio);
	void BloomRender(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gbuffer, RenderTexture& forwardPassText);
	void SSR_Render(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gBuffer);
	void SSAO_Render(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gBuffer);

	void HbaoPlusInit(DX11& gfx11, int width, int height);
	void HbaoPlusRender(DX11& gfx11, RectShape& rect, Camera& camera, ID3D11ShaderResourceView* depthView, ID3D11ShaderResourceView* normalView);

	RenderTexture bloomRenderTexture;
	RenderTexture downSampleTexture, horizontalGaussianBlurTexture, verticalGaussianBlurTexture;

	RenderTexture SsrRenderTexture;
	RenderTexture hbaoTexture;

public:
	float radius;
	float bias;
	float sharpness;
	float powerExponent;
	float metersToViewSpaceUnits;
	float largeScaleAO;
	float smallScaleAO;
	float decodeBias;
	float decodeScale;
private:
	GFSDK_SSAO_Status status;
	GFSDK_SSAO_Context_D3D11* pAOContext;


};

