#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;	//!< カラー(Color)
layout (set=0, binding=1) uniform sampler2D Sampler2D1;	//!< 法線(Normal)
layout (set=0, binding=2) uniform sampler2D Sampler2D2;	//!< 深度(Depth)
layout (set=0, binding=3) uniform sampler2D Sampler2D3; //!< 未定

void main()
{
	OutColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const vec4 Mask[] = {
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	//!< 分割画面ではビューポート1以降を使用しているのでMask[]の添え字と合わせるために-1 (Devided screen use viewport [1, 4])
	const int VPIndex = gl_ViewportIndex - 1;
	
	OutColor.rgb += texture(Sampler2D, InTexcoord).rgb * Mask[VPIndex].xxx;

	OutColor.rgb += texture(Sampler2D1, InTexcoord).rgb * Mask[VPIndex].yyy;

	OutColor.rgb += texture(Sampler2D2, InTexcoord).rrr * Mask[VPIndex].zzz;
	
	OutColor.rgb += texture(Sampler2D3, InTexcoord).rgb * Mask[VPIndex].www;	
}