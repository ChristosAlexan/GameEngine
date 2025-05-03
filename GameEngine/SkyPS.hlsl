

cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
    float bloomBrightness;
    float bloomStrength;
    float ambientStrength;
    float exposure;
    float envMapStrength;
}


cbuffer lightBuffer : register(b6)
{
    float3 color;
    float bEmissive;
}
cbuffer skyBuffer : register(b8)
{
    float4 apexColor;
    float4 centerColor;

}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inWorldPos : WOLRD_POSITION;
};

Texture2D depthTexture : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);
float4 main(PS_INPUT input) : SV_TARGET
{
    float depth = depthTexture.Load(input.inPosition.xyz).z;
    float dist = input.inPosition.z / input.inPosition.w;
    if (dist < depth)
        discard;
    
    
    float3 viewDir = normalize(input.inWorldPos.xyz);
    float zenithAngle = acos(dot(viewDir, float3(0, 1, 0)));
    float3 skyColor = lerp(centerColor.rgb, apexColor.rgb, saturate(1.0 - pow(zenithAngle / 3.14159, 2)));
    skyColor *= envMapStrength;
    return float4(skyColor, 1.0f);
}