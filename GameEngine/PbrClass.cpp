#include "PbrClass.h"
#include "ConstantBuffersGlobals.h"
#include "ShadersGlobals.h"

PbrClass::PbrClass()
{
}

void PbrClass::Initialize(DX11& gfx11)
{
	brdfTexture.Initialize(gfx11.device.Get(), 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void PbrClass::BrdfRender(DX11& gfx11, RectShape& rect, Camera& camera, RenderTexture& texture)
{
	gfx11.deviceContext->RSSetViewports(1, &texture.m_viewport);
	texture.SetRenderTarget(gfx11.deviceContext.Get(), texture.m_depthStencilView.Get());
	texture.ClearRenderTarget(gfx11.deviceContext.Get(), texture.m_depthStencilView.Get(), 0, 0, 0, 1);

	gfx11.deviceContext->PSSetShader(GFX_GLOBALS::brdfPS.GetShader(), nullptr, 0);
	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::vs2D.GetInputLayout());
	gfx11.deviceContext->VSSetShader(GFX_GLOBALS::vs2D.GetShader(), nullptr, 0);
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	rect.Draw(gfx11.deviceContext.Get(), camera, GFX_GLOBALS::cb_vs_vertexshader);
}

void PbrClass::IrradianceConvolutionRender(DX11& gfx11, CubeShape debugCube, EnvironmentProbe& environmentProbe, Camera& camera, float* rgb)
{
	debugCube.pos = DirectX::XMFLOAT3(0, 0, 0);
	environmentProbe.pos = debugCube.pos;
	environmentProbe.UpdateCamera();

	unsigned int _width = 32;
	unsigned int _height = 32;
	unsigned int maxMipLevels = 1;
	unsigned int mip = 0;
	irradianceCubeMap.CreateCubeMap(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, DXGI_FORMAT_R16G16B16A16_FLOAT, maxMipLevels);
	gfx11.deviceContext->RSSetViewports(1, &irradianceCubeMap.m_viewport);

	gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
	gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
	gfx11.deviceContext->PSSetSamplers(2, 1, gfx11.samplerState_MipMap.GetAddressOf());
	gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
	gfx11.deviceContext->OMSetBlendState(gfx11.blendState.Get(), NULL, 0xFFFFFFFF);



	for (int face = 0; face < 6; ++face)
	{
		irradianceCubeMap.RenderCubeMapFace(gfx11.device.Get(), gfx11.deviceContext.Get(), face, mip, gfx11.depthStencilView.Get(), rgb, true);
		gfx11.deviceContext->GenerateMips(irradianceCubeMap.shaderResourceView);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		GFX_GLOBALS::cb_ps_pbrBuffer.data.roughness = roughness;
		GFX_GLOBALS::cb_ps_pbrBuffer.UpdateBuffer();

		gfx11.deviceContext->PSSetShader(GFX_GLOBALS::irradianceConvPS.GetShader(), nullptr, 0);
		gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
		gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredVS.GetInputLayout());
		gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
		gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
		gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
		gfx11.deviceContext->PSSetSamplers(1, 1, gfx11.samplerState_Clamp.GetAddressOf());
		gfx11.deviceContext->PSSetSamplers(2, 1, gfx11.samplerState_MipMap.GetAddressOf());

		DirectX::XMMATRIX viewMatrix = environmentProbe.viewMatrices[face];
		DirectX::XMMATRIX projectionMatrix = environmentProbe.projectionMatrices[face];

		debugCube.SetRenderTexture(gfx11.deviceContext.Get(), environmentProbe.environmentCubeMap);
		gfx11.deviceContext->PSSetShaderResources(0, 1, &environmentProbe.environmentCubeMap.shaderResourceView);
		debugCube.Draw(gfx11.deviceContext.Get(), viewMatrix, projectionMatrix, GFX_GLOBALS::cb_vs_vertexshader);
	}
}

void PbrClass::PrifilterRender(DX11& gfx11, CubeShape debugCube, EnvironmentProbe& environmentProbe, Camera& camera, float* rgb)
{
	debugCube.pos = DirectX::XMFLOAT3(0, 0, 0);
	environmentProbe.pos = debugCube.pos;
	environmentProbe.UpdateCamera();

	unsigned int maxMipLevels = 5;

	unsigned int _width = 128;
	unsigned int _height = 128;
	prefilterCubeMap.CreateCubeMap(gfx11.device.Get(), gfx11.deviceContext.Get(), _width, _height, DXGI_FORMAT_R16G16B16A16_FLOAT, maxMipLevels);

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = static_cast<unsigned int>(_width * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(_height * std::pow(0.5, mip));

		gfx11.deviceContext->RSSetViewports(1, &prefilterCubeMap.m_viewport);

		for (int face = 0; face < 6; ++face)
		{
			prefilterCubeMap.RenderCubeMapFace(gfx11.device.Get(), gfx11.deviceContext.Get(), face, mip, gfx11.depthStencilView.Get(), rgb, true);
			gfx11.deviceContext->GenerateMips(prefilterCubeMap.shaderResourceView);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			GFX_GLOBALS::cb_ps_pbrBuffer.data.roughness = roughness;
			GFX_GLOBALS::cb_ps_pbrBuffer.UpdateBuffer();

			gfx11.deviceContext->PSSetShader(GFX_GLOBALS::prefilterPS.GetShader(), nullptr, 0);
			gfx11.deviceContext->OMSetDepthStencilState(gfx11.depthStencilState.Get(), 0);
			gfx11.deviceContext->IASetInputLayout(GFX_GLOBALS::deferredVS.GetInputLayout());
			gfx11.deviceContext->VSSetShader(GFX_GLOBALS::deferredVS.GetShader(), nullptr, 0);
			gfx11.deviceContext->RSSetState(gfx11.rasterizerState.Get());
			gfx11.deviceContext->PSSetSamplers(0, 1, gfx11.samplerState_Wrap.GetAddressOf());
			gfx11.deviceContext->PSSetSamplers(1, 1, gfx11.samplerState_Clamp.GetAddressOf());
			gfx11.deviceContext->PSSetSamplers(2, 1, gfx11.samplerState_MipMap.GetAddressOf());

			DirectX::XMMATRIX viewMatrix = environmentProbe.viewMatrices[face];
			DirectX::XMMATRIX projectionMatrix = environmentProbe.projectionMatrices[face];

			debugCube.SetRenderTexture(gfx11.deviceContext.Get(), environmentProbe.environmentCubeMap);
			gfx11.deviceContext->PSSetShaderResources(0, 1, &environmentProbe.environmentCubeMap.shaderResourceView);
			debugCube.Draw(gfx11.deviceContext.Get(), viewMatrix, projectionMatrix, GFX_GLOBALS::cb_vs_vertexshader);

		}
	}
}

void PbrClass::PbrRender(DX11& gfx11, RectShape& rect, CubeShape debugCube, EnvironmentProbe& environmentProbe, Camera& camera, float* rgb)
{
	BrdfRender(gfx11, rect,camera, brdfTexture);
	IrradianceConvolutionRender(gfx11,debugCube, environmentProbe,camera,rgb);
	PrifilterRender(gfx11, debugCube, environmentProbe, camera, rgb);
}
