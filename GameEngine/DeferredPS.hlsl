cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
}

cbuffer materialBuffer : register(b6)
{
    float3 color;
    float bEmissive;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal : NORMAL;
    float3 inWorldPos : WORLD_POSITION;
    float3 inTangent : TANGENT;
    float3 inBinormal : BINORMAL;
};

struct PS_OUTPUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 roughnessMetalic : SV_Target2;
    float4 worldPosition : SV_Target3;
    float4 depth : SV_TARGET4;
    float4 ssao_normal : SV_Target5;
};

Texture2D albedoTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D roughnessMetalicTexture : TEXTURE : register(t2);

SamplerState SampleTypeWrap : register(s0);

PS_OUTPUT main(PS_INPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    
   
  
    
    if (bEmissive == 0.0f)
    {
        output.albedo = albedoTexture.Sample(SampleTypeWrap, input.inTexCoord);
        if (output.albedo.a < 0.95)
        {
            discard;
        }
       
        
       output.normal = normalTexture.Sample(SampleTypeWrap, input.inTexCoord);
       //output.ssao_normal = float4(output.normal.rgb, 1.0f);
        
       output.normal = (output.normal * 2.0f) - 1.0f;
       output.ssao_normal = output.normal;
        
       float3 bumpNormal = (output.normal.x * input.inTangent) + (output.normal.y * input.inBinormal) + (output.normal.z * input.inNormal);
       bumpNormal = normalize(bumpNormal);
       output.normal = float4(bumpNormal, 1.0f);
    
       output.roughnessMetalic = roughnessMetalicTexture.Sample(SampleTypeWrap, input.inTexCoord);
    }
    else if(bEmissive == 1.0f)
    {
        output.albedo = float4(color.r, color.g, color.b, 1.0f);
        output.normal = float4(-1, -1, -1, 1.0f);
        output.roughnessMetalic = float4(-1, -1, -1, 1.0f);
    }
    output.worldPosition = float4(input.inWorldPos, 1.0f);
    
    float depthValue = input.inPosition.z / input.inPosition.w;
    output.depth = float4(depthValue, depthValue, depthValue,1.0f);
   
    return output;
}