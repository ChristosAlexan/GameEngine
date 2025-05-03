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
cbuffer cameraBuffer : register(b2)
{
    float4x4 viewMatrix; // View matrix
    float4x4 projectionMatrix; // Projection matrix
    float4x4 viewProjectionMatrix; // Combined view-projection matrix
    float4x4 inverseViewProjectionMatrix;
    float4x4 inverseViewMatrix;
    float4x4 inverseProjectionMatrix; // Inverse projection matrix
    float4 testValues;
    float4 cameraPos;
    float2 screenSize;
};

Texture2D deferredTexture : TEXTURE : register(t0);
Texture2D forwardTexture : TEXTURE : register(t1);

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
    float2 texCoords = input.inPosition.xy / float2(screenSize.x, screenSize.y);
    
    float3 deferredColor = deferredTexture.Sample(objSamplerState, input.inTexCoord);
    float3 forwardColor = forwardTexture.Sample(objSamplerState, texCoords);

    //sampleColor += ssr*0.5;
    //sampleColor = ReinhardToneMapping(sampleColor, exposure);
    //sampleColor = pow(sampleColor, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
    
    return float4(deferredColor + forwardColor, 1.0f);
}