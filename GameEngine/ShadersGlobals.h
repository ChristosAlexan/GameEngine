#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"

namespace GFX_GLOBALS
{
	extern VertexShader vs2D;
	extern VertexShader instancedVS;
	extern VertexShader testVS;
	extern VertexShader pbrVS;
	extern VertexShader volumetricLightVS;
	extern VertexShader animVS;
	extern VertexShader depthAnimVS;
	extern VertexShader depthVS;
	extern VertexShader gaussianBlurVS;
	extern VertexShader deferredVS;
	extern VertexShader animDeferredVS;
	extern VertexShader shadowVS;
	extern VertexShader deferredLightPassVS;
	extern VertexShader ssrVS;

	extern PixelShader alphaBlendPS;
	extern PixelShader testPS;
	extern PixelShader colorPS;
	extern PixelShader lightPS;
	extern PixelShader pbrPS;
	extern PixelShader transparentPbrPS;
	extern PixelShader depthPS;
	extern PixelShader cubeMapPS;
	extern PixelShader irradianceConvPS;
	extern PixelShader brdfPS;
	extern PixelShader prefilterPS;
	extern PixelShader envProbePS;
	extern PixelShader downSampleBlurPS, horizontalGaussianBlurPS, verticalGaussianBlurPS;
	extern PixelShader bloomLightPS;
	extern PixelShader volumetricLightPS;
	extern PixelShader postProccessPS;
	extern PixelShader volumeGPassPS;
	extern PixelShader deferredPS;
	extern PixelShader shadowPS;
	extern PixelShader deferredLightPassPS;
	extern PixelShader deferredSpotLightPS;
	extern PixelShader skyPS;
	extern PixelShader ssrPS;
	extern PixelShader deferredForwardCombinePS;
	extern PixelShader shadowHorizontalGaussianBlurPS, shadowVerticalGaussianBlurPS;
}
