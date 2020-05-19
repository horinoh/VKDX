#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;	//!< カラー(Color)
layout (set=0, binding=1) uniform sampler2D Sampler2D1; //!< 法線(Normal)
layout (set=0, binding=2) uniform sampler2D Sampler2D2;	//!< 深度(Depth)
layout (set=0, binding=3) uniform sampler2D Sampler2D3; //!< 未定

void main()
{
	//!< UVと深度からワールド位置を求める
	//const vec4 Tmp = InvViewProjection * vec4(InTexcoord * 2.0f - 1.0f, texture(Sampler2D2, InTexcoord).r, 1.0f);
	//const vec3 WorldPos = Tmp.xyz / Tmp.w;

	OutColor = texture(Sampler2D, InTexcoord);
}