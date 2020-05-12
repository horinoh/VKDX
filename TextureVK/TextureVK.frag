#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

//!< �A�v������́u�R���o�C���h�C���[�W�T���v���v(�T���v�� + �T���v���h�C���[�W)�Ƃ��ăo�C���f�B���O(0)�ō쐬���Ă����
//!< �u�R���o�C���h�C���[�W�T���v���v�Ƃ��Ďg���ꍇ���A�u�T���v��+�T���v���h�C���[�W�v�Ƃ��Ďg���ꍇ�ł������o�C���f�B���O(0)�Ƀo�C���h����Ă���
#define USE_COMBINED_IMAGE_SAMPLER

#ifdef USE_COMBINED_IMAGE_SAMPLER
layout (set=0, binding=0) uniform sampler2D Sampler2D;
#else
layout (set=0, binding=0) uniform sampler Sampler;
layout (set=0, binding=0) uniform texture2D Texture;
#endif

void main()
{
#ifdef USE_COMBINED_IMAGE_SAMPLER
	OutColor = texture(Sampler2D, InTexcoord);
#else
	OutColor = texture(sampler2D(Texture, Sampler), InTexcoord);
#endif
	//OutColor = vec4(InTexcoord, 0, 1);
}