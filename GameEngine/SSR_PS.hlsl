cbuffer cameraBuffer : register(b2)
{
    float4x4 viewMatrix; // View matrix (transposed in CPU)
    float4x4 projectionMatrix; // Projection matrix (transposed in CPU)
    float4x4 viewProjectionMatrix; // Combined view-projection matrix
    float4x4 inverseViewProjectionMatrix; // Inverse of view-projection matrix
    float4x4 inverseViewMatrix; // Inverse of view matrix
    float4x4 inverseProjectionMatrix; // Inverse of projection matrix
    float4 testValues;
    float4 cameraPos;
    float2 screenSize;
    float nearPlane;
    float farPlane;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION; // Clip space position
    float2 TexCoord : TEXCOORD; // UV coordinates
};

// Input textures and buffers
Texture2D albedoTexture : register(t0); // Scene color (rendered image)
Texture2D normalTexture : register(t1); // G-buffer normals
Texture2D gPosition : register(t3); // Depth buffer
Texture2D sceneDepth : register(t4); // Depth buffer
SamplerState samplerState : register(s0);


static const int MAX_STEPS = 50;
static const float STEP_SIZE = 0.2f;
static const float DEPTH_TOLERANCE = 0.01f;
static const float BIAS = 0.01f;

// Reconstruct view-space position from screen UV and depth
float3 ReconstructViewPos(float2 uv, float depth)
{
    float4 clipPos = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    float4 viewPos = mul(clipPos, inverseProjectionMatrix);
    return viewPos.xyz / viewPos.w;
}

// Main pixel shader
float4 main(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.TexCoord;
    // === Sample G-buffer data ===
    float sampledDepth = sceneDepth.Sample(samplerState, uv).r;
    if (sampledDepth >= 1.0f)
        return albedoTexture.Sample(samplerState, uv); // Skip skybox

    float3 viewPos = ReconstructViewPos(uv, sampledDepth);

    float3 packedNormal = normalTexture.Sample(samplerState, uv).xyz;
    float3 normal = normalize(packedNormal * 2.0f - 1.0f); // Decode normal from [0,1] to [-1,1]

    // Optional: transform normal to view-space (if you want full correctness)
     normal = mul(normal, (float3x3)viewMatrix);

    float3 viewDir = normalize(-viewPos);
    float3 reflectionDir = reflect(viewDir, normal);

    // === Initialize ray ===
    float3 rayPos = viewPos + reflectionDir * BIAS;
    float3 rayStep = reflectionDir * STEP_SIZE;

    bool hitFound = false;
    float2 hitUV = float2(0.0f, 0.0f);

    // === Ray march loop ===
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        rayPos += rayStep;

        // Project to screen space
        float4 projPos = mul(float4(rayPos, 1.0f), projectionMatrix);
        projPos /= projPos.w;
        float2 rayUV = projPos.xy * 0.5f + 0.5f;

        // Screen bounds check
        if (rayUV.x < 0.0f || rayUV.x > 1.0f || rayUV.y < 0.0f || rayUV.y > 1.0f)
            break;

        float sceneDepthAtRay = sceneDepth.Sample(samplerState, rayUV).r;
        float3 rayViewPos = ReconstructViewPos(rayUV, sceneDepthAtRay);

        float depthDiff = rayViewPos.z - rayPos.z;

        if (abs(depthDiff) < DEPTH_TOLERANCE)
        {
            hitFound = true;
            hitUV = rayUV;
            break;
        }
    }

    // === Reflection color ===
    float4 reflectionColor;
    if (hitFound)
    {
        reflectionColor = albedoTexture.Sample(samplerState, hitUV);
    }
    else
    {
        reflectionColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // fallback to black
    }

    // === Original scene color ===
    float4 originalColor = albedoTexture.Sample(samplerState, uv);

    // === Blend reflection with scene ===
    float reflectionStrength = hitFound ? 0.5f : 0.0f;
    return lerp(originalColor, reflectionColor, reflectionStrength);
}