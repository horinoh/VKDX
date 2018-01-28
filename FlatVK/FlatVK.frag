#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in flat vec3 InNormal;
layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	Color = vec4(InNormal * 0.5f + 0.5f, 1.0f);
}