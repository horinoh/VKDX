#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;
layout (set=0, binding=1) uniform sampler2D Sampler2D1;
layout (set=0, binding=2) uniform sampler2D Sampler2D2;
layout (set=0, binding=3) uniform sampler2D Sampler2D3;

void main()
{
	OutColor = texture(Sampler2D, InTexcoord);
}