#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2DArray Sampler2D;
layout (set=0, binding=1) uniform sampler2DArray DistortionMap;

void main()
{
    const vec2 DistortionIndex = texture(DistortionMap, vec3(InTexcoord, 0.0f)).xy;
    if (DistortionIndex.x < 0.0f || DistortionIndex.x > 1.0f || DistortionIndex.y < 0.0f || DistortionIndex.y > 1.0f)
    {
        OutColor = vec4(0.5f, 0.5f, 0.5f, 1);
        return;
    }
    //OutColor = vec4(texture(Sampler2D, vec3(DistortionIndex, 0.0f)).rrr, 1.0f);

	//OutColor = texture(Sampler2D, vec3(fract(InTexcoord.x * 2.0f), InTexcoord.y, 1.0f - step(InTexcoord.x, 0.5f)));
	OutColor = texture(DistortionMap, vec3(fract(InTexcoord.x * 2.0f), InTexcoord.y, 1.0f - step(InTexcoord.x, 0.5f)));
}