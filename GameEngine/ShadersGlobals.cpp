#include "ShadersGlobals.h"

namespace GFX_GLOBALS
{
	VertexShader vs2D;
	VertexShader instancedVS;
	VertexShader testVS;
	VertexShader pbrVS;
	VertexShader volumetricLightVS;
	VertexShader animVS;
	VertexShader depthAnimVS;
	VertexShader depthVS;
	VertexShader gaussianBlurVS;
	VertexShader deferredVS;
	VertexShader animDeferredVS;
	VertexShader shadowVS;
	VertexShader deferredLightPassVS;
	VertexShader ssrVS;

	PixelShader alphaBlendPS;
	PixelShader testPS;
	PixelShader colorPS;
	PixelShader lightPS;
	PixelShader pbrPS;
	PixelShader transparentPbrPS;
	PixelShader depthPS;
	PixelShader cubeMapPS;
	PixelShader irradianceConvPS;
	PixelShader brdfPS;
	PixelShader prefilterPS;
	PixelShader envProbePS;
	PixelShader downSampleBlurPS, horizontalGaussianBlurPS, verticalGaussianBlurPS;
	PixelShader bloomLightPS;
	PixelShader volumetricLightPS;
	PixelShader postProccessPS;
	PixelShader volumeGPassPS;
	PixelShader deferredPS;
	PixelShader shadowPS;
	PixelShader deferredLightPassPS;
	PixelShader deferredSpotLightPS;
	PixelShader skyPS;
	PixelShader ssrPS;
	PixelShader deferredForwardCombinePS;
	PixelShader shadowHorizontalGaussianBlurPS, shadowVerticalGaussianBlurPS;
}
