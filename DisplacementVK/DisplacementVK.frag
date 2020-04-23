#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal;
layout (location = 1) in vec4 InTangent;
layout (location = 2) in vec2 InTexcoord;
layout (location = 3) in vec3 InViewDirection;

layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	Color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}