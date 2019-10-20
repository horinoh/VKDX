#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

//!< �A�v������͒ʏ�ʂ�T���v���ƃT���v���h�C���[�W���쐬����A�V�F�[�_���ł̉��߂݂̂��قȂ�
#define USE_COMBINED_IMAGE_SAMPLER

#ifdef USE_COMBINED_IMAGE_SAMPLER
//!< �R���o�C���h�C���[�W�T���v��(�T���v�� + �T���v���h�C���[�W)
layout (set=0, binding=0) uniform sampler2D Sampler2D;
#else
//!< �T���v��
layout (set=0, binding=0) uniform sampler Sampler;
//!< �T���v���h�C���[�W
layout (set=0, binding=0) uniform texture2D Texture;
#endif

void main()
{
	//OutColor = vec4(InTexcoord, 0.0f, 1.0f);
#ifdef USE_COMBINED_IMAGE_SAMPLER
	OutColor = texture(Sampler2D, InTexcoord);
#else
	OutColor = texture(sampler2D(Texture, Sampler), InTexcoord);
#endif
}