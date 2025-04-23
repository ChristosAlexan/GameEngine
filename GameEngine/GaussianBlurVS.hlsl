cbuffer constantBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer windowParams : register(b2)
{
    float window_width;
    float window_height;
}
struct VS_INPUT
{
    float3 position : POSITION;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{

    
    VS_OUTPUT output;
   
    output.position = mul(float4(input.position.xyz, 1.0f), worldMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = input.tex;

    return output;
}