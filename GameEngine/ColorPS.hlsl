struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};

Texture2D depthTexture : TEXTURE : register(t0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float depth = depthTexture.Load(input.inPosition.xyz).z;

    float dist = input.inPosition.z / input.inPosition.w;
    if (dist < depth)
        discard;
  
    return float4(1,1,0, 1.0);
}