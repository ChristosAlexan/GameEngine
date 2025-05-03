#pragma once

#include "DX11Includes.h"
#include "Camera.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include <vector>
#include "AdapterReader.h"

#include "RectShape.h"
#include "CubeShape.h"
#include "InstancedShape.h"
#include "GFXGui.h"
#include <future>
#include "ModelLoader.h"
#include "RenderTexture.h"

class DX11
{
public:
	DX11();
	bool Initialize(HWND hwnd,Camera& camera, int width, int height);
	
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool initPSShader(PixelShader* pixelShader, Microsoft::WRL::ComPtr<ID3D11Device> device, std::wstring filePath);
	bool initVSShader(VertexShader* vertexShader, Microsoft::WRL::ComPtr<ID3D11Device> device, std::wstring filePath, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT* numElements);

public:
	UINT32 windowWidth, windowHeight;


	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	D3D11_VIEWPORT viewport;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilDeferredPassState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState2D;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStateFront;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> AdditiveBlendState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> NoBlendState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> deferredLightBlendState;


	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_Wrap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_Clamp;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_MipMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_point;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_deferred;


	VertexBuffer<Vertex> vertexBuffer;
	RenderTexture renderTexture;

	bool bFullScreen;
};

