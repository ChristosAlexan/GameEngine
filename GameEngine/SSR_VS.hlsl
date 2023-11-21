//#pragma pack_matrix(row_major)
cbuffer constantBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};
cbuffer inverseMat_constantBuffer : register(b4)
{
    float4x4 invViewMatrix;
    float4x4 invProjectionMatrix;
};
struct VS_INPUT
{
    float3 inPos : POSITION;
    float2 inTexCoord : TEXCOORD;
    //float3 instancePosition : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    //float4 posH : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
   
    float3 viewRay : POSITION;
};
VS_OUTPUT main(VS_INPUT input)
{

    VS_OUTPUT output;
    
    output.outPosition = mul(float4(input.inPos.xyz, 1.0f), worldMatrix);
    //output.outPosition = mul(output.outPosition, viewMatrix);
    output.outPosition = mul(output.outPosition, projectionMatrix);
    
    output.outTexCoord = input.inTexCoord;
    
   // float2 tex = float2((id << 1) & 2, id & 2);
   // //output.posH = float4(tex * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
   // output.viewRay = float3(tex.x, tex.y, 1);
    
    output.viewRay = mul(output.outPosition, invProjectionMatrix).xyz;
    return output;
}