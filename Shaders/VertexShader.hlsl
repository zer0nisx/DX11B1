cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : WORLD_POSITION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // Transform vertex position to world space
    float4 worldPos = mul(float4(input.position, 1.0f), World);
    
    // Transform to view space
    float4 viewPos = mul(worldPos, View);
    
    // Transform to projection space
    output.position = mul(viewPos, Projection);
    
    // Transform normal to world space
    output.normal = normalize(mul(input.normal, (float3x3)World));
    
    // Pass through texture coordinates
    output.texCoord = input.texCoord;
    
    // Pass world position for lighting calculations
    output.worldPos = worldPos.xyz;
    
    return output;
}
