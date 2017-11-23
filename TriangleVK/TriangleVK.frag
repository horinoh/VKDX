#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

#if 0
layout (push_constant) uniform PushConstant
{
	layout(offset = 64) vec4 Color; //!< �����ł̓t���O�����g�V�F�[�_�p�� 64byte �I�t�Z�b�g���Ă��� For fragment shader offset 64 byte in this case
} InPushConstant;
#endif

layout (location = 0) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

//VkPipelineShaderStageCreateInfo.VkSpecializationInfo �ŏ㏑���l���w�肷��A�w�肪�Ȃ��ꍇ�͂��̃f�t�H���g�l���g����
//layout (constant_id = 0) const int IntValue = 0;
//layout (constant_id = 1) const float FloatValue = 0.0f;
//layout (constant_id = 2) const bool BoolValue = false;

void main()
{
#if 0
	OutColor = InPushConstant.Color;
#else
	OutColor = InColor;
#endif
}