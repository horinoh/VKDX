#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec3 InColor;

layout (location = 0) out vec4 OutColor;

void main()
{
	OutColor = vec4(InColor, 1.0f);
}