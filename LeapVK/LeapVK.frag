#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2DArray LeapMap;
layout (set=0, binding=1) uniform sampler2DArray DistortionMap;

void main()
{
    const float ArrayIndex = 1.0f - step(InTexcoord.x, 0.5f);
    const vec2 UV = vec2(fract(InTexcoord.x * 2.0f), InTexcoord.y);
#if 1
    const vec2 DistortionIndex = texture(DistortionMap, vec3(UV, ArrayIndex)).xy;
    if(any(lessThan(DistortionIndex, vec2(0.0f)))){ discard; }
    if(any(greaterThan(DistortionIndex, vec2(1.0f)))){ discard; }
    OutColor = vec4(texture(LeapMap, vec3(DistortionIndex, ArrayIndex)).rrr, 1.0f);
#else
	OutColor = texture(LeapMap, vec3(UV, ArrayIndex));
	//OutColor = texture(DistortionMap, vec3(UV, ArrayIndex));
#endif
}