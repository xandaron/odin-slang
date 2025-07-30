// Demo Vertex and Fragment Shaders
// Vertex shader transforms vertices, Fragment shader creates a colorful gradient effect

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
};

cbuffer VertexConstants : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 worldViewProjection;
    float3 cameraPosition;
    float time;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    
    // Transform position to world space
    float4 worldPosition = mul(float4(input.position, 1.0), worldMatrix);
    output.worldPos = worldPosition.xyz;
    
    // Apply view and projection transformations
    output.position = mul(worldPosition, mul(viewMatrix, projectionMatrix));
    
    // Transform normal to world space (assuming uniform scaling)
    output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));
    
    // Pass through texture coordinates
    output.texCoord = input.texCoord;
    
    // Animate vertex color based on time
    float wave = sin(time * 2.0 + input.position.x * 0.5);
    output.color = input.color * (0.8 + 0.2 * wave);
    
    return output;
}

// Fragment Shader Constants
cbuffer FragmentConstants : register(b1)
{
    float3 lightDirection;
    float lightIntensity;
    float3 lightColor;
    float ambientStrength;
    float3 viewDirection;
};

Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    // Sample the main texture
    float4 texColor = mainTexture.Sample(mainSampler, input.texCoord);
    
    // Basic Phong lighting calculation
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(-lightDirection);
    
    // Ambient lighting
    float3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightColor * lightIntensity;
    
    // Specular lighting (simple Phong)
    float3 viewDir = normalize(viewDirection);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = spec * lightColor * 0.5;
    
    // Create a colorful gradient based on position
    float3 gradient = float3(
        0.5 + 0.5 * sin(input.worldPos.x * 0.1),
        0.5 + 0.5 * sin(input.worldPos.y * 0.1 + 2.0),
        0.5 + 0.5 * sin(input.worldPos.z * 0.1 + 4.0)
    );
    
    // Combine all lighting components
    float3 lighting = ambient + diffuse + specular;
    float4 finalColor = float4((texColor.rgb * input.color.rgb * gradient * lighting), texColor.a * input.color.a);
    
    return finalColor;
}
