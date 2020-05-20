#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (set = 0, binding = 4) uniform Transform { mat4 Projection; mat4 View; mat4 World; mat4 InverseViewProjection; };

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;	//!< �J���[(Color)
layout (set=0, binding=1) uniform sampler2D Sampler2D1; //!< �@��(Normal)
layout (set=0, binding=2) uniform sampler2D Sampler2D2;	//!< �[�x(Depth)
layout (set=0, binding=3) uniform sampler2D Sampler2D3; //!< ����

void main()
{
	//!< UV�Ɛ[�x���烏�[���h�ʒu�����߂�
	const vec2 UVSnorm = vec2(InTexcoord.x, 1.0f - InTexcoord.y) * 2.0f - 1.0f;
	const float Depth = texture(Sampler2D2, InTexcoord).r;
	vec4 WorldPos = InverseViewProjection * vec4(UVSnorm, Depth, 1.0f);
	WorldPos.xyz /= WorldPos.w;
	//OutColor = vec4(WorldPos.x * 0.5f + 0.5f, 0.0f, 0.0f, 1.0f);
	//OutColor = vec4(0.0f, WorldPos.y * 0.5f + 0.5f, 0.0f, 1.0f);
	//OutColor = vec4(0.0f, 0.0f, WorldPos.z * 0.5f + 0.5f, 1.0f);

	OutColor = texture(Sampler2D, InTexcoord);
}