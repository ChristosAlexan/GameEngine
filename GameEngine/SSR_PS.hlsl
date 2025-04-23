cbuffer pointLightBuffer : register(b1)
{
    float4 pointdynamicLightPosition; // Dynamic point light position
    float4 pointdynamicLightColor; // Dynamic point light color
};

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

struct PSInput
{
    float4 Position : SV_POSITION; // Clip space position
    float2 TexCoord : TEXCOORD; // UV coordinates
};

// Input textures and buffers
Texture2D gAlbedo : register(t0); // Scene color (rendered image)
Texture2D gNormal : register(t1); // G-buffer normals
Texture2D gPosition : register(t3); // Depth buffer
Texture2D SceneDepth : register(t4); // Depth buffer
SamplerState samplerState : register(s0);

// Raymarch parameters
#define MAX_RAY_MARCH_STEPS 64
#define RAY_MARCH_EPSILON 0.001
#define REFLECT_RADIUS 10.0f

// Functions to transform screen space coordinates to world space
float4 ScreenToWorld(float2 screenPos)
{
    float4 clipSpacePos = float4(screenPos.x * 2.0f - 1.0f, (1.0f - screenPos.y) * 2.0f - 1.0f, 1.0f, 1.0f);
    float4 worldPos = mul(clipSpacePos, (projectionMatrix));
    worldPos.xyz /= worldPos.w;
    return mul(worldPos, (viewMatrix));
}

// Raymarching function
bool RayMarch(float3 rayOrigin, float3 rayDir, out float3 hitPos, out float3 hitNormal)
{
    hitPos = rayOrigin;
    hitNormal = float3(0, 0, 0);
    
    for (int i = 0; i < MAX_RAY_MARCH_STEPS; i++)
    {
        // Sample the position and normal textures at the current ray march position
        float3 samplePos = hitPos.xyz;
        float3 sampleNormal = gNormal.Sample(samplerState, hitPos.xy).xyz;
        
        // Check if the ray hits a surface (using a simple depth threshold)
        if (length(hitPos.xyz - rayOrigin) > REFLECT_RADIUS)
        {
            hitNormal = sampleNormal;
            return true;
        }
        
        // March the ray forward
        hitPos += rayDir * RAY_MARCH_EPSILON;
    }
    return false;
}

// SSR function
float4 ScreenSpaceReflection(float2 screenPos, float3 viewDir)
{
    // Get the world position and normal at the current pixel
    float4 position = gPosition.Sample(samplerState, screenPos); // G-buffer position texture
    float3 normal = gNormal.Sample(samplerState, screenPos).xyz;
    float3 albedo = gAlbedo.Sample(samplerState, screenPos).xyz;
    
    // Calculate reflection direction using the normal and view direction
    float3 reflectionDir = reflect(viewDir, normal);
    
    // Cast the reflection ray into screen space
    float3 reflectionPos = position.xyz + reflectionDir * REFLECT_RADIUS;
    
    // Raymarch to find the reflected surface
    float3 hitPos, hitNormal;
    if (RayMarch(reflectionPos, reflectionDir, hitPos, hitNormal))
    {
        // Calculate reflection color (could add a fresnel effect or other enhancements here)
        float3 reflectColor = gAlbedo.Sample(samplerState, hitPos.xy).xyz;
        return float4(reflectColor, 1.0f);
    }
    
    // Return a default value if no reflection is found
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

// Main pixel shader
float4 main(float2 screenPos : TEXCOORD) : SV_Target
{
    // Get view direction for SSR calculation (camera position to current pixel position)
    float3 worldPos = ScreenToWorld(screenPos);
    float3 viewDir = normalize(cameraPos.xyz - worldPos.xyz);
    
    // Compute the SSR effect
    return ScreenSpaceReflection(screenPos, viewDir);
}