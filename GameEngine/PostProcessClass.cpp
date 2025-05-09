#include "PostProcessClass.h"
#include "ConstantBuffersGlobals.h"
#include "ShadersGlobals.h"

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

PostProcessClass::PostProcessClass()
{
	radius = 0.005f;
	bias = 0.5f;
	sharpness = 16.f;
	powerExponent = 2.f;
	metersToViewSpaceUnits = 1.0f;
	largeScaleAO = 1.0f;
	smallScaleAO = 1.0f;
	decodeBias = 1.0f;
	decodeScale = 1.0f;
}

void PostProcessClass::Initialize(DX11& gfx11, int width, int height, float aspectRatio)
{
	downSampleTexture.Initialize(gfx11.device.Get(), width/2, height/2, DXGI_FORMAT_R16G16B16A16_FLOAT);
	horizontalGaussianBlurTexture.Initialize(gfx11.device.Get(), width / 2, height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
	verticalGaussianBlurTexture.Initialize(gfx11.device.Get(), width / 2, height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);

	bloomRenderTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	SsrRenderTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
}

void PostProcessClass::BloomRender(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gbuffer, RenderTexture& forwardPassText)
{
	gfx11.deviceContext->RSSetViewports(1, &bloomRenderTexture.m_viewport);
	bloomRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), bloomRenderTexture.m_depthStencilView.Get());
	bloomRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), bloomRenderTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::bloomLightPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &gbuffer.lightPassTexture.shaderResourceView);
	gfx11.deviceContext->PSSetShaderResources(1, 1, &forwardPassText.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);


	//Guassian blur downSample
	gfx11.deviceContext->RSSetViewports(1, &downSampleTexture.m_viewport);
	downSampleTexture.SetRenderTarget(gfx11.deviceContext.Get(), downSampleTexture.m_depthStencilView.Get());
	downSampleTexture.ClearRenderTarget(gfx11.deviceContext.Get(), downSampleTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::downSampleBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &bloomRenderTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	//Guassian blur horizontal
	gfx11.deviceContext->RSSetViewports(1, &horizontalGaussianBlurTexture.m_viewport);
	horizontalGaussianBlurTexture.SetRenderTarget(gfx11.deviceContext.Get(), horizontalGaussianBlurTexture.m_depthStencilView.Get());
	horizontalGaussianBlurTexture.ClearRenderTarget(gfx11.deviceContext.Get(), horizontalGaussianBlurTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::horizontalGaussianBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &downSampleTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);

	//Guassian blur vertical
	gfx11.deviceContext->RSSetViewports(1, &verticalGaussianBlurTexture.m_viewport);
	verticalGaussianBlurTexture.SetRenderTarget(gfx11.deviceContext.Get(), verticalGaussianBlurTexture.m_depthStencilView.Get());
	verticalGaussianBlurTexture.ClearRenderTarget(gfx11.deviceContext.Get(), verticalGaussianBlurTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::gaussianBlurVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::gaussianBlurVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::verticalGaussianBlurPS.GetShader(), NULL, 0);
	gfx11.deviceContext->PSSetShaderResources(0, 1, &horizontalGaussianBlurTexture.shaderResourceView);
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
}

void PostProcessClass::SSR_Render(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gBuffer)
{
	gfx11.deviceContext->RSSetViewports(1, &SsrRenderTexture.m_viewport);
	SsrRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), SsrRenderTexture.m_depthStencilView.Get());
	SsrRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), SsrRenderTexture.m_depthStencilView.Get(), 0, 0, 0, 1.0f);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::ssrVS.GetShader(), NULL, 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::ssrVS.GetInputLayout());
	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::ssrPS.GetShader(), NULL, 0);

	gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[0]);
	gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[1]);
	gfx11.deviceContext->PSSetShaderResources(2, 1, &gBuffer.m_shaderResourceViewArray[2]);
	gfx11.deviceContext->PSSetShaderResources(3, 1, &gBuffer.m_shaderResourceViewArray[3]);
	gfx11.deviceContext->PSSetShaderResources(4, 1, &gBuffer.m_shaderResourceViewArray[4]);
	
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
}

void PostProcessClass::SSAO_Render(DX11& gfx11, RectShape& rect, Camera& camera, GBufferClass& gBuffer)
{
	//gfx11.deviceContext->RSSetViewports(1, &SsrRenderTexture.m_viewport);
	//SsrRenderTexture.SetRenderTarget(gfx11.deviceContext.Get(), SsrRenderTexture.m_depthStencilView);
	//SsrRenderTexture.ClearRenderTarget(gfx11.deviceContext.Get(), SsrRenderTexture.m_depthStencilView, 0, 0, 0, 1.0f);
	//
	//gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState2D.Get(), 0);
	//gfx11.deviceContext->VSSetShader(gfx11.ssrVS.GetShader(), NULL, 0);
	//gfx11.deviceContext->IASetInputLayout(gfx11.ssrVS.GetInputLayout());
	//gfx11.deviceContext->PSSetShader(gfx11.ssrPS.GetShader(), NULL, 0);
	//
	//gfx11.deviceContext->PSSetShaderResources(0, 1, &gBuffer.m_shaderResourceViewArray[0]);
	//gfx11.deviceContext->PSSetShaderResources(1, 1, &gBuffer.m_shaderResourceViewArray[1]);
	//gfx11.deviceContext->PSSetShaderResources(2, 1, &gBuffer.m_shaderResourceViewArray[2]);
	//gfx11.deviceContext->PSSetShaderResources(3, 1, &gBuffer.m_shaderResourceViewArray[3]);
	//gfx11.deviceContext->PSSetShaderResources(4, 1, &gBuffer.m_shaderResourceViewArray[4]);
	//
	//rect.Draw(gfx11.deviceContext.Get(), camera, gfx11.cb_vs_vertexshader);
}

void PostProcessClass::HbaoPlusInit(DX11& gfx11, int width, int height)
{
	
	hbaoTexture.Initialize(gfx11.device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	
	GFSDK_SSAO_CustomHeap customHeap;
	customHeap.new_ = ::operator new;
	customHeap.delete_ = ::operator delete;


	status = GFSDK_SSAO_CreateContext_D3D11(gfx11.device.Get(), &pAOContext, &customHeap);
	assert(status == GFSDK_SSAO_OK);
}

void PostProcessClass::HbaoPlusRender(DX11& gfx11, RectShape& rect, Camera& camera, ID3D11ShaderResourceView* depthView, ID3D11ShaderResourceView* normalView)
{
	GFSDK_SSAO_Parameters Params;
	Params.LargeScaleAO = largeScaleAO;
	Params.SmallScaleAO = smallScaleAO;
	Params.StepCount = GFSDK_SSAO_STEP_COUNT_8;
	Params.DepthClampMode = GFSDK_SSAO_DepthClampMode::GFSDK_SSAO_CLAMP_TO_BORDER;
	Params.Radius = radius;
	Params.Bias = bias;
	Params.PowerExponent = powerExponent;
	Params.Blur.Enable = true;
	Params.Blur.Radius = GFSDK_SSAO_BlurRadius::GFSDK_SSAO_BLUR_RADIUS_4;
	Params.Blur.Sharpness = sharpness;
	Params.DepthStorage = GFSDK_SSAO_FP16_VIEW_DEPTHS;

	GFSDK_SSAO_InputData_D3D11 Input;
	Input.DepthData.DepthTextureType = GFSDK_SSAO_DepthTextureType::GFSDK_SSAO_HARDWARE_DEPTHS;
	Input.DepthData.pFullResDepthTextureSRV = depthView;
	Input.NormalData.Enable = true;

	Input.NormalData.DecodeBias = decodeBias;
	Input.NormalData.DecodeScale = decodeScale;
	Input.NormalData.pFullResNormalTextureSRV = normalView;
	DirectX::XMMATRIX worldMat =  DirectX::XMMatrixRotationRollPitchYawFromVector(camera.GetRotationVector());
	DirectX::XMMATRIX worldView = (camera.GetViewMatrix()) * (worldMat);
	Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
	Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&worldView);
	DirectX::XMMATRIX proj = camera.GetProjectionMatrix();
	Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&proj);
	Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
	Input.DepthData.MetersToViewSpaceUnits = metersToViewSpaceUnits;
	

	GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RenderMask::GFSDK_SSAO_RENDER_AO;

	GFSDK_SSAO_Output_D3D11 Output;
	Output.pRenderTargetView = hbaoTexture.m_renderTargetView.Get();
	Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;
	status = pAOContext->RenderAO(gfx11.deviceContext.Get(), Input, Params, Output, RenderMask);
	assert(status == GFSDK_SSAO_OK);
}

