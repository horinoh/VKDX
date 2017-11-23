#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

#if 0
layout (push_constant) uniform PushConstant
{
	layout(offset = 64) vec4 Color; //!< ここではフラグメントシェーダ用は 64byte オフセットしている For fragment shader offset 64 byte in this case
} InPushConstant;
#endif

layout (location = 0) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

//VkPipelineShaderStageCreateInfo.VkSpecializationInfo で上書き値を指定する、指定がない場合はこのデフォルト値が使われる
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