#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	const vec4 Colors[] =  {
		vec4(1.0f, 0.0f, 0.0f, 1.0f),
		vec4(0.0f, 1.0f, 0.0f, 1.0f),
		vec4(0.0f, 0.0f, 1.0f, 1.0f),
		vec4(1.0f, 1.0f, 0.0f, 1.0f),
	};
	Color = Colors[gl_ViewportIndex];
}