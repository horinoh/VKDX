#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;	//!< �J���[(Color)
layout (set=0, binding=1) uniform sampler2D Sampler2D1;	//!< �@��(Normal)
layout (set=0, binding=2) uniform sampler2D Sampler2D2;	//!< �[�x(Depth)
layout (set=0, binding=3) uniform sampler2D Sampler2D3; //!< ����

void main()
{
	OutColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const vec4 Mask[] = {
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	//!< ������ʂł̓r���[�|�[�g1�ȍ~���g�p���Ă���̂�Mask[]�̓Y�����ƍ��킹�邽�߂�-1 (Devided screen use viewport [1, 4])
	const int VPIndex = gl_ViewportIndex - 1;
	
	OutColor.rgb += texture(Sampler2D, InTexcoord).rgb * Mask[VPIndex].xxx;

	OutColor.rgb += texture(Sampler2D1, InTexcoord).rgb * Mask[VPIndex].yyy;

	OutColor.rgb += texture(Sampler2D2, InTexcoord).rrr * Mask[VPIndex].zzz;
	
	OutColor.rgb += texture(Sampler2D3, InTexcoord).rgb * Mask[VPIndex].www;	
}