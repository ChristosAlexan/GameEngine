#define SIGMA 2.0f

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

float Gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0f * sigma * sigma)) / (sigma * sqrt(6.283185307179586f));
}



float4 main(PS_INPUT input) : SV_TARGET
{

    float GaussianKernel[9] =
    {
        Gaussian(-4.0f, SIGMA),
        Gaussian(-3.0f, SIGMA),
        Gaussian(-2.0f, SIGMA),
        Gaussian(-1.0f, SIGMA),
        Gaussian(0.0f, SIGMA),
        Gaussian(1.0f, SIGMA),
        Gaussian(2.0f, SIGMA),
        Gaussian(3.0f, SIGMA),
        Gaussian(4.0f, SIGMA)
    };
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int2 texSize;
    lowResBloom.GetDimensions(texSize.x, texSize.y);

    for (int i = -4; i <= 4; i++)
    {
        result += lowResBloom.Sample(samplerState, input.uv + float2(0, i) / texSize) * GaussianKernel[i + 4];
    }

    return result;
}