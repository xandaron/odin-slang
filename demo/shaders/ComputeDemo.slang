// Demo Compute Shader
// Performs parallel computation on a 2D texture/buffer

// Input/Output texture
RWTexture2D<float4> outputTexture : register(u0);
Texture2D<float4> inputTexture : register(t0);

// Compute shader constants
cbuffer ComputeConstants : register(b0)
{
    uint2 textureSize;
    float time;
    float intensity;
    float2 center;
    float radius;
    uint frameCount;
};

// Shared memory for local workgroup operations
groupshared float4 sharedData[16][16];

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID
)
{
    uint2 pixelCoord = dispatchThreadId.xy;
    
    // Bounds check
    if (pixelCoord.x >= textureSize.x || pixelCoord.y >= textureSize.y)
        return;
    
    // Normalize coordinates to [0, 1]
    float2 uv = float2(pixelCoord) / float2(textureSize);
    
    // Sample input texture
    float4 inputColor = inputTexture[pixelCoord];
    
    // Create animated ripple effect
    float2 toCenter = uv - center;
    float distance = length(toCenter);
    
    // Animated wave based on distance and time
    float wave = sin(distance * 20.0 - time * 5.0) * 0.5 + 0.5;
    float ripple = smoothstep(0.0, radius, distance) * (1.0 - smoothstep(radius, radius + 0.1, distance));
    
    // Create color based on position and time
    float3 computedColor = float3(
        0.5 + 0.5 * sin(uv.x * 10.0 + time),
        0.5 + 0.5 * sin(uv.y * 10.0 + time * 1.3),
        0.5 + 0.5 * sin((uv.x + uv.y) * 5.0 + time * 0.7)
    );
    
    // Apply wave effect
    computedColor *= wave * intensity;
    
    // Blend with input color
    float3 finalColor = lerp(inputColor.rgb, computedColor, ripple);
    
    // Store in shared memory for potential local operations
    sharedData[groupThreadId.x][groupThreadId.y] = float4(finalColor, 1.0);
    
    // Synchronize threads in workgroup
    GroupMemoryBarrierWithGroupSync();
    
    // Simple blur using shared memory (optional post-processing)
    if (groupThreadId.x > 0 && groupThreadId.x < 15 && 
        groupThreadId.y > 0 && groupThreadId.y < 15)
    {
        float4 blurred = (
            sharedData[groupThreadId.x-1][groupThreadId.y-1] +
            sharedData[groupThreadId.x][groupThreadId.y-1] +
            sharedData[groupThreadId.x+1][groupThreadId.y-1] +
            sharedData[groupThreadId.x-1][groupThreadId.y] +
            sharedData[groupThreadId.x][groupThreadId.y] * 4.0 +
            sharedData[groupThreadId.x+1][groupThreadId.y] +
            sharedData[groupThreadId.x-1][groupThreadId.y+1] +
            sharedData[groupThreadId.x][groupThreadId.y+1] +
            sharedData[groupThreadId.x+1][groupThreadId.y+1]
        ) / 12.0;
        
        // Subtle blur effect
        finalColor = lerp(finalColor, blurred.rgb, 0.1);
    }
    
    // Write final result to output texture
    outputTexture[pixelCoord] = float4(finalColor, 1.0);
}
