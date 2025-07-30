#version 450
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_KHR_memory_scope_semantics : require
layout(column_major) uniform;
layout(column_major) buffer;

#line 16 0
struct SLANG_ParameterGroup_ComputeConstants_std140_0
{
    uvec2 textureSize_0;
    float time_0;
    float intensity_0;
    vec2 center_0;
    float radius_0;
    uint frameCount_0;
};


#line 9
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_ComputeConstants_std140_0
{
    uvec2 textureSize_0;
    float time_0;
    float intensity_0;
    vec2 center_0;
    float radius_0;
    uint frameCount_0;
}ComputeConstants_0;

#line 6
layout(binding = 0)
uniform texture2D inputTexture_0;


#line 5
layout(rgba32f)
layout(binding = 0)
uniform image2D outputTexture_0;


#line 20
shared vec4   sharedData_0[16][16];



layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{



    uvec2 pixelCoord_0 = gl_GlobalInvocationID.xy;

#line 30
    bool _S1;


    if(pixelCoord_0.x >= ComputeConstants_0.textureSize_0.x)
    {

#line 33
        _S1 = true;

#line 33
    }
    else
    {

#line 33
        _S1 = pixelCoord_0.y >= ComputeConstants_0.textureSize_0.y;

#line 33
    }

#line 33
    if(_S1)
    {

#line 34
        return;
    }

    vec2 uv_0 = vec2(pixelCoord_0) / vec2(ComputeConstants_0.textureSize_0);


    ;

#line 40
    ivec3 _S2 = ivec3(uvec3(pixelCoord_0, 0U));



    float distance_0 = length(uv_0 - ComputeConstants_0.center_0);

#line 52
    float _S3 = uv_0.x;
    float _S4 = uv_0.y;

#line 61
    vec3 finalColor_0 = mix((texelFetch((inputTexture_0), ((_S2)).xy, ((_S2)).z)).xyz, vec3(0.5 + 0.5 * sin(_S3 * 10.0 + ComputeConstants_0.time_0), 0.5 + 0.5 * sin(_S4 * 10.0 + ComputeConstants_0.time_0 * 1.29999995231628418), 0.5 + 0.5 * sin((_S3 + _S4) * 5.0 + ComputeConstants_0.time_0 * 0.69999998807907104)) * ((sin(distance_0 * 20.0 - ComputeConstants_0.time_0 * 5.0) * 0.5 + 0.5) * ComputeConstants_0.intensity_0), vec3(smoothstep(0.0, ComputeConstants_0.radius_0, distance_0) * (1.0 - smoothstep(ComputeConstants_0.radius_0, ComputeConstants_0.radius_0 + 0.10000000149011612, distance_0))));


    uint _S5 = gl_LocalInvocationID.x;

#line 64
    uint _S6 = gl_LocalInvocationID.y;

#line 64
    sharedData_0[_S5][_S6] = vec4(finalColor_0, 1.0);


    controlBarrier(gl_ScopeWorkgroup, gl_ScopeWorkgroup, gl_StorageSemanticsShared, gl_SemanticsAcquireRelease);


    if(_S5 > 0U)
    {

#line 70
        _S1 = _S5 < 15U;

#line 70
    }
    else
    {

#line 70
        _S1 = false;

#line 70
    }

#line 70
    if(_S1)
    {

#line 70
        _S1 = _S6 > 0U;

#line 70
    }
    else
    {

#line 70
        _S1 = false;

#line 70
    }
    if(_S1)
    {

#line 71
        _S1 = _S6 < 15U;

#line 71
    }
    else
    {

#line 71
        _S1 = false;

#line 71
    }

#line 71
    vec3 finalColor_1;

#line 70
    if(_S1)
    {


        uint _S7 = _S6 - 1U;

#line 80
        uint _S8 = _S6 + 1U;

#line 80
        finalColor_1 = mix(finalColor_0, ((sharedData_0[_S5 - 1U][_S7] + sharedData_0[_S5][_S7] + sharedData_0[_S5 + 1U][_S7] + sharedData_0[_S5 - 1U][_S6] + sharedData_0[_S5][_S6] * 4.0 + sharedData_0[_S5 + 1U][_S6] + sharedData_0[_S5 - 1U][_S8] + sharedData_0[_S5][_S8] + sharedData_0[_S5 + 1U][_S8]) / 12.0).xyz, vec3(0.10000000149011612));

#line 70
    }
    else
    {

#line 70
        finalColor_1 = finalColor_0;

#line 70
    }

#line 90
    imageStore((outputTexture_0), (ivec2(pixelCoord_0)), vec4(finalColor_1, 1.0));
    return;
}

