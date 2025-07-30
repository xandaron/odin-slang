#version 460
layout(column_major) uniform;
layout(column_major) buffer;

#line 1400 0
struct _MatrixStorage_float4x4std140_0
{
    vec4  data_0[4];
};


#line 28 1
struct SLANG_ParameterGroup_VertexConstants_std140_0
{
    _MatrixStorage_float4x4std140_0 worldMatrix_0;
    _MatrixStorage_float4x4std140_0 viewMatrix_0;
    _MatrixStorage_float4x4std140_0 projectionMatrix_0;
    _MatrixStorage_float4x4std140_0 worldViewProjection_0;
    vec3 cameraPosition_0;
    float time_0;
};


#line 21
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_VertexConstants_std140_0
{
    _MatrixStorage_float4x4std140_0 worldMatrix_0;
    _MatrixStorage_float4x4std140_0 viewMatrix_0;
    _MatrixStorage_float4x4std140_0 projectionMatrix_0;
    _MatrixStorage_float4x4std140_0 worldViewProjection_0;
    vec3 cameraPosition_0;
    float time_0;
}VertexConstants_0;

#line 21
mat4x4 unpackStorage_0(_MatrixStorage_float4x4std140_0 _S1)
{

#line 21
    return mat4x4(_S1.data_0[0][0], _S1.data_0[0][1], _S1.data_0[0][2], _S1.data_0[0][3], _S1.data_0[1][0], _S1.data_0[1][1], _S1.data_0[1][2], _S1.data_0[1][3], _S1.data_0[2][0], _S1.data_0[2][1], _S1.data_0[2][2], _S1.data_0[2][3], _S1.data_0[3][0], _S1.data_0[3][1], _S1.data_0[3][2], _S1.data_0[3][3]);
}


#line 11979 2
layout(location = 0)
out vec3 entryPointParam_vertexMain_worldPos_0;


#line 11979
layout(location = 1)
out vec3 entryPointParam_vertexMain_normal_0;


#line 4 1
layout(location = 2)
out vec2 entryPointParam_vertexMain_texCoord_0;


#line 4
layout(location = 3)
out vec4 entryPointParam_vertexMain_color_0;


#line 4
layout(location = 0)
in vec3 input_position_0;


#line 4
layout(location = 1)
in vec3 input_normal_0;


#line 4
layout(location = 2)
in vec2 input_texCoord_0;


#line 4
layout(location = 3)
in vec4 input_color_0;


#line 12
struct VertexOutput_0
{
    vec4 position_0;
    vec3 worldPos_0;
    vec3 normal_0;
    vec2 texCoord_0;
    vec4 color_0;
};


#line 32
void main()
{



    vec4 worldPosition_0 = (((unpackStorage_0(VertexConstants_0.worldMatrix_0)) * (vec4(input_position_0, 1.0))));

#line 34
    VertexOutput_0 output_0;



    output_0.worldPos_0 = worldPosition_0.xyz;


    output_0.position_0 = ((((((unpackStorage_0(VertexConstants_0.projectionMatrix_0)) * (unpackStorage_0(VertexConstants_0.viewMatrix_0))))) * (worldPosition_0)));

#line 41
    mat4x4 _S2 = unpackStorage_0(VertexConstants_0.worldMatrix_0);


    output_0.normal_0 = normalize((((mat3x3(_S2[0].xyz, _S2[1].xyz, _S2[2].xyz)) * (input_normal_0))));


    output_0.texCoord_0 = input_texCoord_0;



    output_0.color_0 = input_color_0 * (0.80000001192092896 + 0.20000000298023224 * sin(VertexConstants_0.time_0 * 2.0 + input_position_0.x * 0.5));

    VertexOutput_0 _S3 = output_0;

#line 53
    gl_Position = output_0.position_0;

#line 53
    entryPointParam_vertexMain_worldPos_0 = _S3.worldPos_0;

#line 53
    entryPointParam_vertexMain_normal_0 = _S3.normal_0;

#line 53
    entryPointParam_vertexMain_texCoord_0 = _S3.texCoord_0;

#line 53
    entryPointParam_vertexMain_color_0 = _S3.color_0;

#line 53
    return;
}

