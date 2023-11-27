#define NO_LIGHTS 16

cbuffer lightBuffer : register(b0)
{
    float4 dynamicLightPosition[NO_LIGHTS];
    float4 dynamicLightColor[NO_LIGHTS];
    float4 SpotlightDir[NO_LIGHTS];
    float4 cameraPos;
    float4 lightTypeEnableShadows[NO_LIGHTS];
    float4x4 lightViewMatrix[NO_LIGHTS];
    float4x4 lightProjectionMatrix[NO_LIGHTS];
    uint lightsSize;
}

cbuffer shadowsbuffer : register(b9)
{
    float4 shadowsSoftnessBias[NO_LIGHTS];
    double bias;
}
cbuffer ssrBuffer : register(b10)
{
    float4x4 ssrViewMatrix;
    float4x4 ssrProjectionMatrix;
    float4x4 ssrInvViewMatrix;
    float4x4 ssrInvProjectionMatrix;
    float4 ssrCameraPos;
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
Texture2D distToCameraTexture : TEXTURE : register(t4);
TextureCube prefilterMap : TEXTURE : register(t5);
Texture2D brdfTexture : TEXTURE : register(t6);
TextureCube irradianceMap : TEXTURE : register(t7);
Texture2D uvTexture : TEXTURE : register(t8);
Texture2D depthMapTextures[NO_LIGHTS] : TEXTURE : register(t9);
//Texture2D depthMapTextures[NO_LIGHTS] : TEXTURE : register(t7);

SamplerState SampleTypeWrap : register(s0);
SamplerState SampleTypeClamp : register(s1);
SamplerState objSamplerStateMip : SAMPLER : register(s2);

float3 Shadows(float4 lightViewPosition, Texture2D depthMapTexture, PS_INPUT input, int index);

float3 fresnelSchlick(float cosTheta, float3 F0);
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 pointLight(PS_INPUT input, float3 albedo, float3 pos, float3 color, float4 _cutOff, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos);
float3 spotLight(PS_INPUT input, float3 albedo, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos, int index);
float3 dirLight(PS_INPUT input, float3 albedo, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos, int index);

float4 main(PS_INPUT input) : SV_TARGET
{
    int3 sampleIndices = int3(input.inPosition.xy, 0);
    
    float4 albedo = objTexture.Sample(SampleTypeWrap, input.inTexCoord);
    //float4 albedo = float4(pow(objTexture.Sample(SampleTypeWrap, input.inTexCoord), gamma));
    float3 bumpNormal = normalTexture.Sample(SampleTypeWrap, input.inTexCoord).rgb;
    if (bumpNormal.r == -1 && bumpNormal.g == -1 && bumpNormal.b == -1)
    {
        float3 color = albedo.rgb;
        return float4(color, 1.0f);
    }
    float metallic = roughnessMetalicTexture.Sample(SampleTypeWrap, input.inTexCoord).b;
    float roughness = roughnessMetalicTexture.Sample(SampleTypeWrap, input.inTexCoord).g;
    float4 worldPos = worldPositionTexture.Load(sampleIndices);
    
    
    float3 V = normalize(cameraPos.xyz - worldPos.xyz);

    float3 ambient = float3(0.1, 0.1, 0.1);
    float3 F0 = float3(0.04f, 0.04f, 0.04f);

    F0 = lerp(F0, albedo.rgb, metallic);
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    if (lightsSize > 0)
    {
        [unroll(NO_LIGHTS)]
        for (int i = 0; i < NO_LIGHTS; ++i)
        {
            if (i > lightsSize - 1)
                break;
            
            float distance = length(dynamicLightPosition[i].xyz - worldPos.xyz);
            if (distance < RadiusAndcutOff[i].x)
            {
                if (lightTypeEnableShadows[i].x == 0.0)
                    Lo += pointLight(input, albedo.rgb, dynamicLightPosition[i].xyz, dynamicLightColor[i].rgb, RadiusAndcutOff[i].y, bumpNormal, roughness, metallic, V, F0, worldPos.xyz);
                else if (lightTypeEnableShadows[i].x == 1.0)
                {
                    if (lightTypeEnableShadows[i].y == 1.0f)
                        Lo += spotLight(input, albedo.rgb, bumpNormal, roughness, metallic, V, F0, worldPos.xyz, i) * Shadows(worldPos, depthMapTextures[i],input,i);
                    else
                        Lo += spotLight(input, albedo.rgb, bumpNormal, roughness, metallic, V, F0, worldPos.xyz, i);

                }
                else if (lightTypeEnableShadows[i].x == 2.0)
                {
                    if (lightTypeEnableShadows[i].y == 1.0f)
                    {
                        if(i==0)
                            Lo += dirLight(input, albedo.rgb, bumpNormal, roughness, metallic, V, F0, worldPos.xyz, i) * Shadows(worldPos, depthMapTextures[i], input, i);
                        else
                            Lo *= Shadows(worldPos, depthMapTextures[i], input, i);
                    }
                    else
                        Lo += dirLight(input, albedo.rgb, bumpNormal, roughness, metallic, V, F0, worldPos.xyz, i);
                }

            }
        }
      
    }
    
    float3 F = fresnelSchlickRoughness(max(dot(bumpNormal, V), 0.0f), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0f - kS;
    float3 irradiance = irradianceMap.Sample(objSamplerStateMip, bumpNormal.rgb).rgb;
    float3 diffuse = irradiance * albedo.rgb;
    float3 R = reflect(-V, bumpNormal);
    
    
    const float MAX_REF_LOD = 5.0f;
    float3 prefilteredColor = prefilterMap.SampleLevel(objSamplerStateMip, R, roughness * MAX_REF_LOD).rgb;
    float2 brdf = brdfTexture.Sample(SampleTypeWrap, float2(max(dot(bumpNormal, V), 0.0), roughness)).rg;

    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    ambient = (kD * diffuse + specular) * ambientStrength;
    float3 color = ambient + Lo;
   
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

float3 pointLight(PS_INPUT input, float3 albedo, float3 pos, float3 color, float4 _cutOff, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos)
{
    float3 L = normalize(pos.xyz - worldPos.xyz).xyz;
    
    //float theta = dot(L, normalize(-lightDirectionAndSpecularPower[index].xyz));
    float outerCutOff = _cutOff.x / 3.0f;
    float epsilon = _cutOff.x - outerCutOff;
    float intensity = clamp((L - outerCutOff) / epsilon, 0.0f, 1.0f);
    
    float3 H = normalize(V + L);
        
    float distance = length(pos.xyz - worldPos.xyz);
    //float attenuation = 1.0f / (distance * distance);
    float attenuation = 1.0f / (distance * distance) * epsilon;
    float3 radiance = color.xyz * attenuation;

        
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

float3 spotLight(PS_INPUT input, float3 albedo, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos, int index)
{
    float3 L = normalize(dynamicLightPosition[index].xyz - worldPos.xyz).xyz;
    float theta = dot(L, normalize(-SpotlightDir[index].xyz));
    float outerCutOff = RadiusAndcutOff[index].y / 3.0f;
    float epsilon = RadiusAndcutOff[index].y - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0f, 1.0f);
    
    float3 H = normalize(V + L);
        
    float distance = length(dynamicLightPosition[index].xyz - worldPos.xyz);
    float attenuation = 1.0f / (distance * distance) * intensity;
    float3 radiance = dynamicLightColor[index].xyz * attenuation;

        
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

float3 dirLight(PS_INPUT input, float3 albedo, float3 bumpNormal, float roughness, float metallic, float3 V, float3 F0, float3 worldPos, int index)
{
    float3 L = normalize(-SpotlightDir[index].xyz);
  
    float3 H = normalize(V + L);
        
    float3 radiance = dynamicLightColor[index].xyz;

        
    float NDF = DistributionGGX(bumpNormal, H, roughness);
    float G = GeometrySmith(bumpNormal, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
  
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


float3 Shadows(float4 worldPos, Texture2D depthMapTexture, PS_INPUT input, int index)
{
    float4 lightViewPosition = worldPos;
    lightViewPosition = mul(lightViewPosition, transpose(lightViewMatrix[index]));
    lightViewPosition = mul(lightViewPosition, transpose(lightProjectionMatrix[index]));
    
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
        
        if(lightTypeEnableShadows[index].x == 2.0f)
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
