#pragma once
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"

namespace GFX_GLOBALS
{
	//CONSTANT BUFFERS
	extern ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	extern ConstantBuffer<CB_VS_ssrBuffer> cb_vs_ssrBuffer;
	extern ConstantBuffer<CB_VS_lightsShader> cb_vs_lightsShader;
	extern ConstantBuffer<CB_VS_windowParams> cb_vs_windowParams;
	extern ConstantBuffer<CB_VS_instanceShader> cb_vs_instanceShader;
	extern ConstantBuffer<CB_VS_inverseCoordsBuffer> cb_vs_inverseCoordsBuffer;

	extern ConstantBuffer<CB_PS_lightsShader> cb_ps_lightsShader;
	extern ConstantBuffer<CB_PS_pointLightsShader> cb_ps_pointLightsShader;
	extern ConstantBuffer<CB_PS_lightCull> cb_ps_lightCull;
	extern ConstantBuffer<CB_PS_pointLightCull> cb_ps_pointLightCull;
	extern ConstantBuffer<CB_PS_screenEffectBuffer> cb_ps_screenEffectBuffer;
	extern ConstantBuffer<CB_PS_pbrBuffer> cb_ps_pbrBuffer;
	extern ConstantBuffer<CB_PS_materialBuffer> cb_ps_materialBuffer;
	extern ConstantBuffer<CB_PS_cameraBuffer> cb_ps_cameraBuffer;
	extern ConstantBuffer<CB_PS_skyBuffer> cb_ps_skyBuffer;
	extern ConstantBuffer<CB_PS_shadowsBuffer> cb_ps_shadowsBuffer;
	extern ConstantBuffer<CB_PS_inverseCoordsBuffer> cb_ps_inverseCoordsBuffer;
	extern ConstantBuffer<CB_PS_ssaoBuffer> cb_ps_ssaoBuffer;
}


