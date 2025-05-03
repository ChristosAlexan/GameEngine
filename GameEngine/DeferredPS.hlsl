cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
}

cbuffer materialBuffer : register(b6)
{
    float3 color;
    float bEmissive;
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
    float nearPlane;
    float farPlane;
};

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
    float4 specularEmmision : SV_Target5;
};

Texture2D albedoTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D roughnessMetalicTexture : TEXTURE : register(t2);

SamplerState SampleTypeWrap : register(s0);

PS_OUTPUT main(PS_INPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    
  
    output.albedo = albedoTexture.Sample(SampleTypeWrap, input.inTexCoord);
    if (output.albedo.a < 0.95)
    {
        discard;
    }
    if (bEmissive)
    {
        output.specularEmmision = float4(1, 1, 1, 1);
        output.albedo = float4(1, 1, 1, 1);

    }
        
    else
        output.specularEmmision = float4(0, 0, 0, 0);


    float3 tangentNormal = normalTexture.Sample(SampleTypeWrap, input.inTexCoord).xyz;
    tangentNormal = tangentNormal * 2.0f - 1.0f; // Unpack from [0,1] to [-1,1]
    // Build TBN matrix (must come from vertex shader)
    float3x3 TBN = float3x3(input.inTangent, input.inBinormal, input.inNormal);
    // Transform tangent space normal to world space
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    output.normal = float4(worldNormal, 1.0f);

    
    output.roughnessMetalic = roughnessMetalicTexture.Sample(SampleTypeWrap, input.inTexCoord);


    output.worldPosition = float4(input.inWorldPos, 1.0f);
    
    float depth = (input.inPosition.z / input.inPosition.w);
    output.depth = float4(depth, depth, depth, 1.0f);
   
    
    return output;
}