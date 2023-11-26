struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};
cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
    float bloomBrightness;
    float bloomStrength;
    float ambinetStrength;
    float exposure;
}

Texture2D objTexture : TEXTURE : register(t0);
Texture2D bloomTexture : TEXTURE : register(t1);
Texture2D forwardRenderTexture : TEXTURE : register(t2);
Texture2D hbaoPlusTexture : TEXTURE : register(t3);

SamplerState objSamplerState : SAMPLER : register(s0);
SamplerState objSamplerStateClamp : SAMPLER : register(s1);




float3 ReinhardToneMapping(float3 color, float exposure)
{
    float3 mappedColor = color / (color + 1.0);
    mappedColor = pow(mappedColor, float3(1.0 / exposure, 1.0 / exposure, 1.0 / exposure));
    return mappedColor;
}


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 sampleColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    float3 bloom = bloomTexture.Sample(objSamplerState, input.inTexCoord);
    float3 forwardColor = forwardRenderTexture.Sample(objSamplerState, input.inTexCoord);
    float3 hbaoPlus = hbaoPlusTexture.Sample(objSamplerState, input.inTexCoord).rgb;
  
    sampleColor *= hbaoPlus;
    sampleColor += bloom * bloomStrength;
    //sampleColor += forwardColor;
    sampleColor = ReinhardToneMapping(sampleColor, exposure);
    sampleColor = pow(sampleColor, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
    
    return float4(sampleColor, 1.0f);
}