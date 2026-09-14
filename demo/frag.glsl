#version 450
layout(column_major) uniform;
layout(column_major) buffer;

#line 66 0
layout(binding = 0)
uniform texture2D mainTexture_0;


#line 67
layout(binding = 0)
uniform sampler mainSampler_0;


#line 57
struct SLANG_ParameterGroup_FragmentConstants_0
{
    vec3 lightDirection_0;
    float lightIntensity_0;
    vec3 lightColor_0;
    float ambientStrength_0;
    vec3 viewDirection_0;
};


#line 57
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_FragmentConstants_0
{
    vec3 lightDirection_0;
    float lightIntensity_0;
    vec3 lightColor_0;
    float ambientStrength_0;
    vec3 viewDirection_0;
}FragmentConstants_0;

#line 18
layout(location = 0)
out vec4 entryPointParam_fragmentMain_0;


#line 18
layout(location = 0)
in vec3 input_worldPos_0;


#line 18
layout(location = 1)
in vec3 input_normal_0;


#line 18
layout(location = 2)
in vec2 input_texCoord_0;


#line 18
layout(location = 3)
in vec4 input_color_0;


#line 70
void main()
{

    vec4 texColor_0 = (texture(sampler2D(mainTexture_0,mainSampler_0), (input_texCoord_0)));


    vec3 normal_0 = normalize(input_normal_0);
    vec3 lightDir_0 = normalize(- FragmentConstants_0.lightDirection_0);

#line 77
    entryPointParam_fragmentMain_0 = vec4(texColor_0.xyz * input_color_0.xyz * vec3(0.5 + 0.5 * sin(input_worldPos_0.x * 0.10000000149011612), 0.5 + 0.5 * sin(input_worldPos_0.y * 0.10000000149011612 + 2.0), 0.5 + 0.5 * sin(input_worldPos_0.z * 0.10000000149011612 + 4.0)) * (FragmentConstants_0.ambientStrength_0 * FragmentConstants_0.lightColor_0 + max(dot(normal_0, lightDir_0), 0.0) * FragmentConstants_0.lightColor_0 * FragmentConstants_0.lightIntensity_0 + pow(max(dot(normalize(FragmentConstants_0.viewDirection_0), reflect(- lightDir_0, normal_0)), 0.0), 32.0) * FragmentConstants_0.lightColor_0 * 0.5), texColor_0.w * input_color_0.w);

#line 77
    return;
}

