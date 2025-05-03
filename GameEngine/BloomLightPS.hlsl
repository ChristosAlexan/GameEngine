cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
    float bloomBrightness;
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

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
   // float3 inNormal : NORMAL;
   // float3 inWorldPos : WOLRD_POSITION;
};

Texture2D deferredTexture : TEXTURE : register(t0);
Texture2D forwardTexture : TEXTURE : register(t1);

SamplerState objSamplerState : SAMPLER : register(s0);
SamplerState objSamplerStateClamp : SAMPLER : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 texCoords = input.inPosition.xy / float2(screenSize.x, screenSize.y);
    float3 sampleColor = deferredTexture.Sample(objSamplerState, input.inTexCoord);
    //sampleColor += forwardTexture.Sample(objSamplerState, texCoords);
    //float brightness = dot(sampleColor, float3(0.2126, 0.7152, 0.0722));
    float brightness = dot(sampleColor, float3(0.2126, 0.2126, 0.2126));
    if (brightness > bloomBrightness)
    {
        return float4(sampleColor.x, sampleColor.y, sampleColor.z, 1.0f);
    }
    else
    {
        return float4(0, 0, 0, 1.0f);
    }
}