cbuffer screenEffectBuffer : register(b4)
{
    float gamma;
    float bloomBrightness;
    float bloomStrength;
    float ambientStrength;
    float exposure;
    float envMapStrength;
}

cbuffer pointLightBuffer : register(b1)
{
    float4 pointdynamicLightPosition;
    float4 pointdynamicLightColor;
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

cbuffer pointLightCull : register(b7)
{
    float4 pointRadiusAndcutOff;
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

SamplerState SampleTypeWrap : register(s0);
SamplerState objSamplerStateMip : SAMPLER : register(s2);

float3 fresnelSchlick(float cosTheta, float3 F0);
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 pointLight(PS_INPUT input, float3 albedo, float3 pos, float3 color, float4 _radiuscutOff, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos);
float3 ReinhardToneMapping(float3 color, float exposure)
{
    float3 mappedColor = color / (color + 1.0);
    mappedColor = pow(mappedColor, float3(1.0 / exposure, 1.0 / exposure, 1.0 / exposure));
    return mappedColor;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    
    float2 texCoords = input.inPosition.xy / float2(screenSize.x, screenSize.y);
    float4 bumpNormal = normalTexture.Sample(SampleTypeWrap, texCoords);
    float emission = specularEmmission.Sample(SampleTypeWrap, texCoords).a;
    
    if (dot(bumpNormal, bumpNormal) < 0.001)
        discard;

    bumpNormal = normalize(bumpNormal);
    
    float4 albedo = objTexture.Sample(SampleTypeWrap, texCoords);
    
       

    float metallic = roughnessMetalicTexture.Sample(SampleTypeWrap, texCoords).b;
    float roughness = roughnessMetalicTexture.Sample(SampleTypeWrap, texCoords).g;
    float3 worldPos = worldPositionTexture.Sample(SampleTypeWrap, texCoords).xyz;
    
    if (emission == 1.0f)
    {
        bumpNormal = float4(0.577f, 0.577f, 0.577f, 1);
        metallic = 0;
        roughness = 0;

    }
  
    float3 V = normalize(cameraPos.xyz - worldPos);

    float3 ambient = float3(0.1, 0.1, 0.1);
    float3 F0 = float3(0.04f, 0.04f, 0.04f);

    F0 = lerp(F0, albedo.rgb, metallic);
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    Lo = pointLight(input, albedo.rgb, pointdynamicLightPosition.xyz, pointdynamicLightColor.rgb, pointRadiusAndcutOff, bumpNormal.xyz, roughness, metallic, V, F0, worldPos);
    
    
    
    float3 color = Lo;
    
    color = ReinhardToneMapping(color, exposure);
    color = pow(color, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));

    return float4(color, 1.0);
   
}



float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0, 1.0), 5.0f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    
    denom = PI * denom * denom;
    return nom / denom;
    //return nom / max(denom, 0.001f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float3 pointLight(PS_INPUT input, float3 albedo, float3 pos, float3 color, float4 _radiuscutOff, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos)
{
    float3 L = normalize(pos.xyz - worldPos.xyz).xyz;
    
    float outerCutOff = _radiuscutOff.y / 3.0f;
    float epsilon = _radiuscutOff.y - outerCutOff;
    float intensity = clamp((L - outerCutOff) / epsilon, 0.0f, 1.0f);
    
    float3 H = normalize(V + L);
 
    float distance = length(pos - worldPos);
    float attenuation = 1.0f / (distance * distance) * epsilon;
    float3 radiance = color.xyz * attenuation;
    float radius = _radiuscutOff.x; // .x is the radius
    if (distance > radius)
        discard;
    
    float NDF = DistributionGGX(bumpNormal, H, roughness);
    float G = GeometrySmith(bumpNormal, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
    //float3 F = fresnelSchlickRoughness(max(dot(bumpNormal, V), 0.0f), F0, roughness);

        
    float3 nominator = NDF * G * F;
    float denominator = 4 * max(dot(bumpNormal, V), 0.0f) * max(dot(bumpNormal, L), 0.0f) + 0.001;
    float3 specular = (nominator / denominator);
        
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
        
    float NdotL = max(dot(bumpNormal, L), 0.0f);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}


float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
    //return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}