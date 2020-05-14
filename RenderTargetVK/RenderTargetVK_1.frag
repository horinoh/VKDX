#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;

void main()
{
#if 0
	const vec2 UV = InTexcoord;	
#elif 0
	//!< ラスター
	const float PI = 4.0f * atan(1.0f);
	const vec2 UV = InTexcoord + vec2(0.05f * cos(InTexcoord.y * PI * 10.0f), 0.0f);
#else
	//!< モザイク
	const vec2 Resolution = vec2(800.0f, 600.0f);
	const float Block = 10.0f;
	const vec2 UV = floor(InTexcoord * Resolution / Block) * Block / Resolution;
#endif

	OutColor = texture(Sampler2D, UV);
}