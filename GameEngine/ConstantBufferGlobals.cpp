#include "ConstantBuffersGlobals.h"

namespace GFX_GLOBALS
{
	//CONSTANT BUFFERS
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_VS_ssrBuffer> cb_vs_ssrBuffer;
	ConstantBuffer<CB_VS_lightsShader> cb_vs_lightsShader;
	ConstantBuffer<CB_VS_windowParams> cb_vs_windowParams;
	ConstantBuffer<CB_VS_instanceShader> cb_vs_instanceShader;
	ConstantBuffer<CB_VS_inverseCoordsBuffer> cb_vs_inverseCoordsBuffer;

	ConstantBuffer<CB_PS_lightsShader> cb_ps_lightsShader;
	ConstantBuffer<CB_PS_pointLightsShader> cb_ps_pointLightsShader;
	ConstantBuffer<CB_PS_lightCull> cb_ps_lightCull;
	ConstantBuffer<CB_PS_pointLightCull> cb_ps_pointLightCull;
	ConstantBuffer<CB_PS_screenEffectBuffer> cb_ps_screenEffectBuffer;
	ConstantBuffer<CB_PS_pbrBuffer> cb_ps_pbrBuffer;
	ConstantBuffer<CB_PS_materialBuffer> cb_ps_materialBuffer;
	ConstantBuffer<CB_PS_cameraBuffer> cb_ps_cameraBuffer;
	ConstantBuffer<CB_PS_skyBuffer> cb_ps_skyBuffer;
	ConstantBuffer<CB_PS_shadowsBuffer> cb_ps_shadowsBuffer;
	ConstantBuffer<CB_PS_inverseCoordsBuffer> cb_ps_inverseCoordsBuffer;
	ConstantBuffer<CB_PS_ssaoBuffer> cb_ps_ssaoBuffer;
}