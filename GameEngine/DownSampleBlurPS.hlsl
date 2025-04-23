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
    float2 tex : TEXCOORD;
};

Texture2D bloomExtract : TEXTURE : register(t0);
SamplerState samplerState : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 _screenSize = float2(screenSize.x / 2, screenSize.y / 2);
    float4 sum = 0;
	    // Sample 4 pixels around the center (Box filter)
    float2 texelSize = float2(1.0 / _screenSize.x, 1.0 / _screenSize.y); // Assuming 1920x1080 resolution
    
    sum += bloomExtract.Sample(samplerState, input.tex + texelSize * float2(-0.5, -0.5));
    sum += bloomExtract.Sample(samplerState, input.tex + texelSize * float2(0.5, -0.5));
    sum += bloomExtract.Sample(samplerState, input.tex + texelSize * float2(-0.5, 0.5));
    sum += bloomExtract.Sample(samplerState, input.tex + texelSize * float2(0.5, 0.5));

    return sum * 0.25; // Average the samples
}