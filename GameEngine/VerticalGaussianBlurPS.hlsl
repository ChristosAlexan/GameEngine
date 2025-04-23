cbuffer cameraBuffer : register(b2)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewProjectionMatrix;
    float4x4 inverseViewMatrix;
    float4x4 inverseProjectionMatrix;
    float4 testValues;
    float4 cameraPos;
    float2 screenSize;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D lowResBloom : TEXTURE : register(t0);
SamplerState samplerState : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
   float2 texelSize = float2(1.0 / (screenSize.x / 2), 1.0 / (screenSize.y / 2));

    // Use a softer, wider blur kernel (with more evenly spread weights)
    float weights[9] = { 0.125, 0.135, 0.125, 0.110, 0.090, 0.070, 0.050, 0.030, 0.010 };

    float4 sum = lowResBloom.Sample(samplerState, input.uv) * weights[0];

    for (int i = 1; i < 9; i++)
    {
        sum += lowResBloom.Sample(samplerState, input.uv + texelSize * float2(i, 0)) * weights[i];
        sum += lowResBloom.Sample(samplerState, input.uv - texelSize * float2(i, 0)) * weights[i];
        sum += lowResBloom.Sample(samplerState, input.uv + texelSize * float2(0, i)) * weights[i];
        sum += lowResBloom.Sample(samplerState, input.uv - texelSize * float2(0, i)) * weights[i];
    }

    return sum;
}