cbuffer ConstantBuffer : register(b0)
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4x4 g_InvProj;
    float4x4 g_TangentToWorld;
    float3 g_CameraPos;
    float2 g_ViewportSize;
    float g_SSRMaxDistance;
    float g_RayOffset;
    float g_SampleRadius;
    float g_FocalLength;
    float g_FresnelPower;
    float g_WeightThreshold;
    float g_DepthScale;
    float g_FalloffBias;
    float g_FalloffScale;
    float g_SpecularPower;
    float g_SpecularScale;
    float g_ReflectionIntensity;
};

Texture2D<float> g_DepthTexture : register(t0);
Texture2D<float4> g_NormalTexture : register(t1);
Texture2D<float4> g_DiffuseTexture : register(t2);
SamplerState g_LinearWrapSampler : register(s0);

float4 SampleSphere(int index)
{
    switch (index)
    {
        case 0:
            return float4(1.0, 0.0, 0.0, 0.0);
        case 1:
            return float4(-1.0, 0.0, 0.0, 0.0);
        case 2:
            return float4(0.0, 1.0, 0.0, 0.0);
        case 3:
            return float4(0.0, -1.0, 0.0, 0.0);
        case 4:
            return float4(0.0, 0.0, 1.0, 0.0);
        case 5:
            return float4(0.0, 0.0, -1.0, 0.0);
        default:
            return float4(0.0, 0.0, 0.0, 0.0);
    }
}

float3 Falloff(float distance, float bias, float scale)
{
    float3 result = 1.0 / (bias + scale * distance);
    return saturate(result);
}

float3 Specular(float3 reflection, float3 halfVec, float depthDifference, float power, float scale)
{
    float fresnelTerm = pow(saturate(1.0 - dot(halfVec, reflection)), power);
    float specularTerm = saturate((depthDifference + scale) / scale);
    return fresnelTerm * specularTerm;
}

float Fresnel(float cosTheta, float power)
{
    return pow(1.0 - saturate(cosTheta), power);
}

float4 main(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float2 texelSize = 1.0 / g_ViewportSize;

    float3 viewPos = mul(float4(vpos.xyz, 1.0), g_InvProj).xyz;
    float2 screenPos = vpos.xy / vpos.w;
    float3 viewDir = normalize(viewPos);

    float3 normal = g_NormalTexture.Sample(g_LinearWrapSampler, texcoord).xyz;
    float3 worldNormal = normalize(mul(normal, g_TangentToWorld).xyz);

    float3 viewPosOffset = viewPos + worldNormal * g_RayOffset;
    float3 reflectionPos = viewPosOffset - g_SSRMaxDistance * viewDir;

    float3 color = float3(0.0, 0.0, 0.0);

    float3 totalWeight = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < 6; ++i)
    {
        float3 ray = normalize(mul(SampleSphere(i), g_View).xyz - viewPosOffset);
        float t = 0.0;
        float3 pos = float3(0.0, 0.0, 0.0);
        float3 normal = float3(0.0, 0.0, 0.0);
        bool hit = false;

        for (int j = 0; j < 64; ++j)
        {
            pos = viewPosOffset + t * ray;
            float depth = g_DepthTexture.Sample(g_LinearWrapSampler, pos.xy * texelSize).r;
            if (depth <= pos.z)
            {
                hit = true;
                break;
            }

            normal = g_NormalTexture.Sample(g_LinearWrapSampler, pos.xy * texelSize).xyz;
            t += g_DepthScale * (depth - pos.z);
            if (t > g_SSRMaxDistance)
            {
                break;
            }
        }

        if (!hit)
        {
            continue;
        }

        float3 rayPos = viewPosOffset + t * ray;
        float3 hitPos = rayPos + g_DepthScale * normal;
        float3 diffColor = g_DiffuseTexture.Sample(g_LinearWrapSampler, hitPos.xy * texelSize).xyz;

        float3 halfVec = normalize(viewDir + ray);
        float3 reflection = normalize(reflect(-ray, normal));
        float3 falloff = Falloff(length(rayPos - viewPos), g_FalloffBias, g_FalloffScale);
        float fresnel = Fresnel(dot(reflection, -viewDir), g_FresnelPower);
        float3 specular = Specular(reflection, halfVec, length(viewPos - rayPos), g_SpecularPower, g_SpecularScale);
        float weight = dot(falloff, float3(1.0, 1.0, fresnel)) * g_ReflectionIntensity;

        float3 finalColor = weight * diffColor * specular;
        color += finalColor;
        totalWeight += weight;
    }

    if (totalWeight.x < g_WeightThreshold && totalWeight.y < g_WeightThreshold && totalWeight.z < g_WeightThreshold)
    {
        return g_DiffuseTexture.Sample(g_LinearWrapSampler, texcoord);
    }

    return float4(color / totalWeight, 1.0);
}