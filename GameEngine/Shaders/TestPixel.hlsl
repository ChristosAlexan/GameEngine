struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord :TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord).xyz;
	return float4(pixelColor,1.0);
}