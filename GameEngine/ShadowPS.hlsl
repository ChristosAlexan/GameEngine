#define NO_LIGHTS 16

cbuffer lightBuffer : register(b0)
{
    float4 dynamicLightPosition[NO_LIGHTS];
    float4 dynamicLightColor[NO_LIGHTS];
    float4 SpotlightDir[NO_LIGHTS];
    float4 lightTypeEnableShadows[NO_LIGHTS];
    float4x4 lightViewMatrix[NO_LIGHTS];
    float4x4 lightProjectionMatrix[NO_LIGHTS];
    uint lightsSize;
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

cbuffer shadowsbuffer : register(b9)
{
    float4 shadowsSoftnessBias[NO_LIGHTS];
    float bias;
}

cbuffer lightCull : register(b3)
{
    float4 RadiusAndcutOff[NO_LIGHTS];
}

cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
    float bloomBrightness;
    float bloomStrength;
    float ambientStrength;
    float exposure;
    float envMapStrength;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};

static const float PI = 3.14159265359;

Texture2D objTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D roughnessMetalicTexture : TEXTURE : register(t2);
Texture2D worldPositionTexture : TEXTURE : register(t3);
Texture2D depthTexture : TEXTURE : register(t4);
Texture2D specularEmmission : TEXTURE : register(t5);
TextureCube prefilterMap : TEXTURE : register(t6);
Texture2D brdfTexture : TEXTURE : register(t7);
TextureCube irradianceMap : TEXTURE : register(t8);
Texture2D depthMapTextures[NO_LIGHTS] : TEXTURE : register(t9);

SamplerState SampleTypeWrap : register(s0);
SamplerState SampleTypeClamp : register(s1);
SamplerState objSamplerStateMip : SAMPLER : register(s2);

float3 Shadows(float4 lightViewPosition, Texture2D depthMapTexture, PS_INPUT input, int index);




float4 main(PS_INPUT input) : SV_TARGET
{
    uint2 textureSize;
    // Retrieve dimensions of the world position texture
    worldPositionTexture.GetDimensions(textureSize.x, textureSize.y);
    
    int2 sampleIndices = int2(input.inTexCoord * float2(textureSize.x, textureSize.y));

    float4 worldPos = worldPositionTexture.Load(int3(sampleIndices, 0));
    
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    if (lightsSize > 0)
    {
        for (int i = 0; i < 1; ++i)
        {
            if (i > lightsSize - 1)
                break;
            Lo += Shadows(worldPos, depthMapTextures[i], input, i);

          
        }
    }

    return float4(Lo, 1.0);
}

float3 Shadows(float4 worldPos, Texture2D depthMapTexture, PS_INPUT input, int index)
{
    float4 lightViewPosition = worldPos;
    lightViewPosition = mul(lightViewPosition, lightViewMatrix[index]);
    lightViewPosition = mul(lightViewPosition, lightProjectionMatrix[index]);
    
    float shadow = 0.0f;
    int width;
    int height;
    depthMapTexture.GetDimensions(width, height);
    float2 texelSize;
    texelSize.x = shadowsSoftnessBias[index].x / width;
    texelSize.y = shadowsSoftnessBias[index].x / height;
    
    float3 projCoords;
   
    projCoords.x = lightViewPosition.x / lightViewPosition.w * 0.5 + 0.5;
    projCoords.y = -lightViewPosition.y / lightViewPosition.w * 0.5 + 0.5;
    projCoords.z = lightViewPosition.z / lightViewPosition.w;
    
    
    projCoords.z = projCoords.z - bias * shadowsSoftnessBias[index].y;
    if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y))
    {
        int PCF_RANGE = 2;
        int SUN_PCF_RANGE = 4;
        
        if (lightTypeEnableShadows[index].x == 2.0f)
        {
            [unroll(SUN_PCF_RANGE*2+1)]
            for (int x = -SUN_PCF_RANGE; x <= SUN_PCF_RANGE; x++)
            {
            [unroll(SUN_PCF_RANGE*2+1)]
                for (int y = -SUN_PCF_RANGE; y <= SUN_PCF_RANGE; y++)
                {
                    float pcfDepth = depthMapTexture.Sample(SampleTypeWrap, projCoords.xy + float2(x, y) * texelSize).r;
             
                    shadow += projCoords.z > pcfDepth ? 0.0f : 1.0f;
                }
            }
            shadow /= ((SUN_PCF_RANGE * 2 + 1) * (SUN_PCF_RANGE * 2 + 1));
        }
        else
        {
            [unroll(PCF_RANGE*2+1)]
            for (int x = -PCF_RANGE; x <= PCF_RANGE; x++)
            {
            [unroll(PCF_RANGE*2+1)]
                for (int y = -PCF_RANGE; y <= PCF_RANGE; y++)
                {
                    float pcfDepth = depthMapTexture.Sample(SampleTypeWrap, projCoords.xy + float2(x, y) * texelSize).r;
             
                    shadow += projCoords.z > pcfDepth ? 0.0f : 1.0f;
                }
            }
            shadow /= ((PCF_RANGE * 2 + 1) * (PCF_RANGE * 2 + 1));
        }
        
    }
    else
    {
        shadow = 1.0f;
    }
    return (float3(shadow, shadow, shadow));
}
