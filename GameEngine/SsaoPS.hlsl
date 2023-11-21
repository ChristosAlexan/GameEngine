cbuffer SSAObuffer : register(b2)
{
    float4x4 projection;
    float4 ssaoKernel[64];
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
    float4x4 projection : TEXCOORD1;
};

Texture2D normalTexture : TEXTURE : register(t0);
Texture2D positionTexture : TEXTURE : register(t1);
Texture2D noiseTexture : TEXTURE : register(t2);

SamplerState objSamplerState : register(s0);
SamplerState objSamplerStateClamp : register(s1);
SamplerState objSamplerStatePoint : register(s3);

const float2 noiseScale = float2(1600.0 / 4.0, 900.0 / 4.0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float radius = 0.5f;
    float bias = 0.025;
    
    //int3 sampleIndices = int3(input.inPosition.xy, 0);
    
   // float3 fragPos = positionTexture.Load(sampleIndices).xyz;
   // float3 normal = normalTexture.Load(sampleIndices).rgb;
   float3 fragPos = positionTexture.Sample(objSamplerState, input.inTexCoord).xyz;
   float3 normal = normalTexture.Sample(objSamplerState, input.inTexCoord).rgb;
    
    if (normal.r == -1 && normal.g == -1 && normal.b == -1)
    {
        return float4(1, 1, 1, 1.0f);
    }
    
    float3 randomVec = noiseTexture.Sample(objSamplerState, input.inTexCoord * noiseScale).xyz;
    
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for (int i = 0; i < 64; ++i)
    {
    // get sample position
        float3 samplePos = mul(TBN, float3(ssaoKernel[i].x, ssaoKernel[i].y, ssaoKernel[i].z)); // from tangent to view-space
        samplePos = fragPos + samplePos * radius;
        
        float4 offset = float4(samplePos, 1.0);
        offset = mul(projection, offset); // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        float sampleDepth = positionTexture.Sample(objSamplerState, offset.xy).z; // get depth value of kernel sample
    
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / 64);
    
    return float4(occlusion, occlusion, occlusion, 1.0f);

}