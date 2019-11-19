#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec3 InTangent;
layout (location = 1) in vec3 InNormal;

layout (location = 0) out vec4 OutColor;

void main()
{
	//OutColor = vec4(normalize(InTangent) * 0.5f + 0.5f, 1.0f);
	OutColor = vec4(normalize(InNormal) * 0.5f + 0.5f, 1.0f);
}