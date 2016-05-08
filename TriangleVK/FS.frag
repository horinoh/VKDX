#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) out vec4 Color;

in IN
{
	vec4 Position;
	vec4 Color;
} In;

void main()
{
	Color = In.Color;
}