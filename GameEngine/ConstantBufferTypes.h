#pragma once

#include <DirectXMath.h>

#define NO_LIGHTS 16

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;

	DirectX::XMMATRIX bones_transform[100];
};


struct CB_VS_inverseCoordsBuffer
{
	DirectX::XMMATRIX invProjectionMatrix;
	DirectX::XMMATRIX invViewMatrix;
};

struct CB_VS_lightsShader
{
	DirectX::XMMATRIX lightViewMatrix[NO_LIGHTS];
	DirectX::XMMATRIX lightProjectionMatrix[NO_LIGHTS];
	unsigned int lightsSize;
};

struct CB_VS_windowParams
{
	float window_width;
	float window_height;
};

struct CB_VS_instanceShader
{
	DirectX::XMFLOAT3 pos[4000];
};

struct CB_PS_lightsShader
{
	DirectX::XMFLOAT4 dynamicLightPosition[NO_LIGHTS];
	DirectX::XMFLOAT4 dynamicLightColor[NO_LIGHTS];
	DirectX::XMFLOAT4 SpotlightDir[NO_LIGHTS];
	DirectX::XMFLOAT4 cameraPos;
	DirectX::XMFLOAT4 lightType[NO_LIGHTS];
	DirectX::XMMATRIX lightViewMatrix[NO_LIGHTS];
	DirectX::XMMATRIX lightProjectionMatrix[NO_LIGHTS];
	unsigned int lightsSize;
};


struct CB_PS_pointLightsShader
{
	DirectX::XMFLOAT4 dynamicLightPosition;
	DirectX::XMFLOAT4 dynamicLightColor;
	DirectX::XMFLOAT4 cameraPos;
};
struct CB_PS_pointLightCull
{
	DirectX::XMFLOAT4 RadiusAndcutOff;
};


struct CB_PS_lightCull
{
	DirectX::XMFLOAT4 RadiusAndcutOff[NO_LIGHTS];
};

struct CB_PS_screenEffectBuffer
{
	float gamma;
	float bloomBrightness;
	float bloomStrength;
	float ambientStrength;

	float exposure;
	float envMapStrength;
	float banding2;
	float banding3;
};

struct CB_PS_pbrBuffer
{
	float roughness;
};

struct CB_PS_materialBuffer
{
	DirectX::XMFLOAT3 emissiveColor;
	float bEmissive;
};

struct CB_PS_ssaoBuffer
{
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMFLOAT4 ssaoKernel[64];
};

struct CB_PS_skyBuffer
{
	DirectX::XMFLOAT4 apexColor;
	DirectX::XMFLOAT4 centerColor;
};

struct CB_PS_shadowsBuffer
{
	DirectX::XMFLOAT4 shadowsSoftnessBias[NO_LIGHTS];
	double bias;
};

struct CB_PS_inverseCoordsBuffer
{
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX viewMatrix;

	DirectX::XMMATRIX invProjectionMatrix;
	DirectX::XMMATRIX invViewMatrix;

	DirectX::XMFLOAT4 cameraPos;
};