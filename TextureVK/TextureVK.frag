#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

//!< �T���v��
//layout (set=0, binding=0) uniform sampler Sampler;
//!< �T���v���h�C���[�W
//layout (set=0, binding=0) uniform texture2D Texture;

//!< �R���o�C���h�C���[�W�T���v��(�T���v�� + �T���v���h�C���[�W) ... �A�v������͒ʏ�ʂ�T���v���ƃT���v���h�C���[�W���쐬����A�V�F�[�_���ł̉��߂݂̂��قȂ�
layout (set=0, binding=0) uniform sampler2D Sampler2D;

void main()
{
	//OutColor = vec4(InTexcoord, 0.0f, 1.0f);
	OutColor = texture(Sampler2D, InTexcoord);
}