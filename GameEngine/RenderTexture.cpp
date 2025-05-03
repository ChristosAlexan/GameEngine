#include "RenderTexture.h"
#include <ScreenGrab.h>
#include <DirectXHelpers.h>
#include <wincodec.h>

RenderTexture::RenderTexture()
{
	//m_renderTargetTexture.Get() = nullptr;
	//m_renderTargetView = nullptr;
	//shaderResourceView = nullptr;
	//m_depthStencilView = nullptr;
}
bool RenderTexture::Initialize(ID3D11Device* device, int width, int height, DXGI_FORMAT format, bool bMapData, D3D11_SUBRESOURCE_DATA* data)
{
	try
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		HRESULT hr;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		if (!init)
		{
			if (bMapData)
				hr = device->CreateTexture2D(&textureDesc, data, m_renderTargetTexture.GetAddressOf());
			else
				hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());

			COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");


		// Setup the viewport for rendering.
		m_viewport.Width = (float)width;
		m_viewport.Height = (float)height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}
bool RenderTexture::InitializeShadow(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int textureWidth, int textureHeight, DXGI_FORMAT format, bool bMapData, D3D11_SUBRESOURCE_DATA* data)
{
	try
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		HRESULT hr;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = textureWidth;
		textureDesc.Height = textureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		if (!init)
		{
			hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = -1;
		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");

		deviceContext->GenerateMips(shaderResourceView);


		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = textureWidth;
		depthBufferDesc.Height = textureHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		//depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		if (!init)
		{
			hr = device->CreateTexture2D(&depthBufferDesc, NULL, &m_texture);
			COM_ERROR_IF_FAILED(hr, "Failed to create depthStencilBuffer");
			init = true;
		}
		// Initailze the depth stencil view description.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDesc, &m_depthStencilView);
		COM_ERROR_IF_FAILED(hr, "Failed to create CreateDepthStencilView");

		// Setup the viewport for rendering.
		m_viewport.Width = (float)textureWidth;
		m_viewport.Height = (float)textureHeight;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}

bool RenderTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext,  int textureWidth, int textureHeight, DXGI_FORMAT format, bool bMapData, D3D11_SUBRESOURCE_DATA* data)
{
	try
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		HRESULT hr;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = textureWidth;
		textureDesc.Height = textureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		if (!init)
		{
			hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = -1;
		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");
		
		deviceContext->GenerateMips(shaderResourceView);


		// Setup the viewport for rendering.
		m_viewport.Width = (float)textureWidth;
		m_viewport.Height = (float)textureHeight;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}

bool RenderTexture::InitializeCustom(ID3D11Device* device, int textureWidth, int textureHeight, DXGI_FORMAT format, float* data)
{
	try
	{
		D3D11_SUBRESOURCE_DATA _data;
		_data.pSysMem = data;
		_data.SysMemPitch = 4 * 12;
		_data.SysMemSlicePitch = 0;

		D3D11_TEXTURE2D_DESC textureDesc;
		HRESULT hr;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = textureWidth;
		textureDesc.Height = textureHeight;
		textureDesc.MipLevels = -1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		hr = device->CreateTexture2D(&textureDesc, &_data, m_renderTargetTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create texture");



		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = -1;
		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");




		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = textureWidth;
		depthBufferDesc.Height = textureHeight;
		depthBufferDesc.MipLevels = -1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		hr = device->CreateTexture2D(&depthBufferDesc, NULL, &m_texture);
		COM_ERROR_IF_FAILED(hr, "Failed to create depthStencilBuffer");

		// Initailze the depth stencil view description.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDesc, &m_depthStencilView);
		COM_ERROR_IF_FAILED(hr, "Failed to create CreateDepthStencilView");

		// Setup the viewport for rendering.
		m_viewport.Width = (float)textureWidth;
		m_viewport.Height = (float)textureHeight;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}

void RenderTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView)
{
	deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), depthStencilView);
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float red, float green, float blue, float alpha, bool bClearStencil)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// Clear the depth buffer.
	if(bClearStencil)
		deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
	else
		deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTexture::GetShaderResourceView()
{
	return shaderResourceView;
}














////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



bool RenderTexture::CreateCubeMap(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned int& width, unsigned int& height, DXGI_FORMAT format, unsigned int& maxMipLevels)
{
	try
	{
		texElementDesc.Width = width;
		texElementDesc.Height = height;
		texElementDesc.MipLevels = maxMipLevels;
		texElementDesc.ArraySize = 6;
		texElementDesc.Format = format;
		texElementDesc.Usage = D3D11_USAGE_DEFAULT;
		texElementDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texElementDesc.SampleDesc.Count = 1;
		texElementDesc.SampleDesc.Quality = 0;
		texElementDesc.CPUAccessFlags = 0;
		texElementDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HRESULT hr = device->CreateTexture2D(&texElementDesc, NULL, m_renderTargetTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create cube map texture");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texElementDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = maxMipLevels;

		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &srvDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view");
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		return false;
	}

	return true;
}

//void RenderTexture::CreateCubeMapMipLevels(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned int& width, unsigned int& height, unsigned int& mipLevel)
//{
//	ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
//	renderTargetDesc.Format = texElementDesc.Format;
//	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
//	renderTargetDesc.Texture2DArray.ArraySize = 1;
//	renderTargetDesc.Texture2DArray.MipSlice = mipLevel;
//
//
//	m_viewport.Width = width;
//	m_viewport.Height = height;
//	m_viewport.MinDepth = 0.0f;
//	m_viewport.MaxDepth = 1.0f;
//	m_viewport.TopLeftX = 0.0f;
//	m_viewport.TopLeftY = 0.0f;
//}

bool RenderTexture::RenderCubeMapFace(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int index, unsigned int& mipLevel, ID3D11DepthStencilView* depthView,float* rgba, bool copyDepth,bool bSave)
{
	try
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texElementDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.FirstArraySlice = index;
		rtvDesc.Texture2DArray.MipSlice = mipLevel;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		HRESULT hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &rtvDesc, &rtv);
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view");

		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(std::max(1u, texElementDesc.Width >> mipLevel));
		viewport.Height = static_cast<float>(std::max(1u, texElementDesc.Height >> mipLevel));
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		deviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), copyDepth ? depthView : nullptr);
		deviceContext->RSSetViewports(1, &viewport);

		deviceContext->ClearRenderTargetView(rtv.Get(), rgba);
		if (copyDepth && depthView)
		{
			deviceContext->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		return false;
	}

	return true;
}

void RenderTexture::RenderAllCubeFaces(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	ID3D11DepthStencilView* depthView,
	float* rgba,
	bool copyDepth)
{
	for (UINT mip = 0; mip < texElementDesc.MipLevels; ++mip)
	{
		for (UINT face = 0; face < 6; ++face)
		{
			RenderCubeMapFace(device, deviceContext, face, mip, depthView, rgba, copyDepth);
		}
	}

	deviceContext->GenerateMips(shaderResourceView);
}
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////




bool RenderTexture::CubeMapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, DXGI_FORMAT format, std::vector<ID3D11Texture2D*>& textureViews)
{
	try
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC texElementDesc;
		texElementDesc.Format = format;
		//texElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureViews[0]->GetDesc(&texElementDesc);


		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = texElementDesc.Width;
		textureDesc.Height = texElementDesc.Height;
		textureDesc.MipLevels = texElementDesc.MipLevels;
		textureDesc.ArraySize = 6;
		textureDesc.Format = texElementDesc.Format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		


		device->GetImmediateContext(&deviceContext);
		D3D11_BOX sourceRegion;
		for (UINT x = 0; x < 6; x++)
		{
			for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
			{
				sourceRegion.left = 0;
				sourceRegion.right = (textureDesc.Width >> mipLevel);
				sourceRegion.top = 0;
				sourceRegion.bottom = (textureDesc.Height >> mipLevel);
				sourceRegion.front = 0;
				sourceRegion.back = 1;
				if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
					break;
				deviceContext->CopySubresourceRegion(m_renderTargetTexture.Get(), D3D11CalcSubresource(mipLevel, x, textureDesc.MipLevels), 0, 0, 0, (ID3D11Resource*)textureViews[x], mipLevel, &sourceRegion);

			}
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
		hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
		COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");

		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = texElementDesc.Width;
		depthBufferDesc.Height = texElementDesc.Height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 6;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (!init)
		{
			hr = device->CreateTexture2D(&depthBufferDesc, NULL, &m_texture);
			init = true;
		}

		COM_ERROR_IF_FAILED(hr, "Failed to create depthStencilBuffer");

		// Initailze the depth stencil view description.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDesc, &m_depthStencilView);
		COM_ERROR_IF_FAILED(hr, "Failed to create CreateDepthStencilView");

		// Setup the viewport for rendering.
		m_viewport.Width = (float)texElementDesc.Width;
		m_viewport.Height = (float)texElementDesc.Height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}

bool RenderTexture::CubeMapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<ID3D11Resource*>& textureViews)
{
	try
	{
		HRESULT hr;


		D3D11_TEXTURE2D_DESC texElementDesc;
		((ID3D11Texture2D*)textureViews[0])->GetDesc(&texElementDesc);

		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = texElementDesc.Width;
		textureDesc.Height = texElementDesc.Height;
		textureDesc.MipLevels = texElementDesc.MipLevels;
		textureDesc.ArraySize = 6;
		textureDesc.Format = texElementDesc.Format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		if (!initCubeMap)
		{
			hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		
		}


		device->GetImmediateContext(&deviceContext);
		D3D11_BOX sourceRegion;
		for (UINT x = 0; x < 6; x++)
		{
			for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
			{
				sourceRegion.left = 0;
				sourceRegion.right = (textureDesc.Width >> mipLevel);
				sourceRegion.top = 0;
				sourceRegion.bottom = (textureDesc.Height >> mipLevel);
				sourceRegion.front = 0;
				sourceRegion.back = 1;
				if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
					break;
				deviceContext->CopySubresourceRegion(m_renderTargetTexture.Get(), D3D11CalcSubresource(mipLevel, x, textureDesc.MipLevels), 0, 0, 0, (ID3D11Resource*)textureViews[x], mipLevel, &sourceRegion);

			}
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		if (!initCubeMap)
		{
			hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
			COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");
			
		}
		

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
		if (!initCubeMap)
		{
			hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
			COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");
		}

		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = texElementDesc.Width;
		depthBufferDesc.Height = texElementDesc.Height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 6;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (!initCubeMap)
		{
			hr = device->CreateTexture2D(&depthBufferDesc, NULL, &m_texture);
			
			COM_ERROR_IF_FAILED(hr, "Failed to create depthStencilBuffer");
		}

		

		// Initailze the depth stencil view description.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		if (!initCubeMap)
		{
			hr = device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDesc, &m_depthStencilView);
			COM_ERROR_IF_FAILED(hr, "Failed to create CreateDepthStencilView");
			initCubeMap = true;
		}
	

		// Setup the viewport for rendering.
		m_viewport.Width = (float)texElementDesc.Width;
		m_viewport.Height = (float)texElementDesc.Height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}

bool RenderTexture::CubeMapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<ID3D11Texture2D*>& textureViews, float width, float height, DXGI_FORMAT format, UINT mip)
{
	try
	{
		HRESULT hr;



		D3D11_TEXTURE2D_DESC texElementDesc;
		texElementDesc.Format = format;
		//texElementDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureViews[0]->GetDesc(&texElementDesc);


		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = texElementDesc.MipLevels;
		textureDesc.ArraySize = 6;
		textureDesc.Format = texElementDesc.Format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		if (!init)
		{
			hr = device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to create texture");
		}


		device->GetImmediateContext(&deviceContext);
		D3D11_BOX sourceRegion;
		for (UINT x = 0; x < 6; x++)
		{
			for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
			{
				sourceRegion.left = 0;
				sourceRegion.right = (textureDesc.Width >> mipLevel);
				sourceRegion.top = 0;
				sourceRegion.bottom = (textureDesc.Height >> mipLevel);
				sourceRegion.front = 0;
				sourceRegion.back = 1;
				if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
					break;
				deviceContext->CopySubresourceRegion(m_renderTargetTexture.Get(), D3D11CalcSubresource(mipLevel, x, textureDesc.MipLevels), 0, 0, 0, (ID3D11Resource*)textureViews[x], mipLevel, &sourceRegion);

			}
		}


		//Setup the description of the render target view
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		if (!init)
		{
			hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView);
			COM_ERROR_IF_FAILED(hr, "Failed to CreateRenderTargetView");
		}

		//Setup the description of the shader resource view
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;

		if (!init)
		{

			hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &shaderResourceView);
			COM_ERROR_IF_FAILED(hr, "Failed to CreateShaderResourceView");
		}

		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = texElementDesc.Width;
		depthBufferDesc.Height = texElementDesc.Height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 6;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (!init)
		{
			hr = device->CreateTexture2D(&depthBufferDesc, NULL, &m_texture);
			
		}

		COM_ERROR_IF_FAILED(hr, "Failed to create depthStencilBuffer");

		// Initailze the depth stencil view description.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		if (!init)
		{
			hr = device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDesc, &m_depthStencilView);
			COM_ERROR_IF_FAILED(hr, "Failed to create CreateDepthStencilView");
			init = true;
		}

		// Setup the viewport for rendering.
		m_viewport.Width = (float)texElementDesc.Width;
		m_viewport.Height = (float)texElementDesc.Height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;


	}
	catch (COMException exc)
	{
		ErrorLogger::Log(exc);
		return false;
	}

	return true;
}



void RenderTexture::SetRenderTargets(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, int count)
{
	std::vector<ID3D11RenderTargetView*> rawRTVs(count);
	for (int i = 0; i < count; ++i)
		rawRTVs[i] = m_renderTargetViews[i].Get();
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(count, rawRTVs.data(), depthStencilView);
	rawRTVs.clear();
}

void RenderTexture::ClearRenderTargets(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, int count, float* rgb)
{
	for (int i = 0; i < count; ++i)
	{
		// Clear the back buffer.
		deviceContext->ClearRenderTargetView(m_renderTargetViews[i].Get(), rgb);

		// Clear the depth buffer.
		deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

}
