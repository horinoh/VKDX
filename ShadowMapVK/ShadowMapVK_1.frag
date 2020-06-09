#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec4 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;
//!< sampler2DShadow を使用するには、サンプラ作成時に比較方法(compareEnable=VK_TRUE, VK_COMPARE_OP_...)を指定すること
layout (set=0, binding=0) uniform sampler2DShadow Sampler2DShadow;

void main()
{
	//OutColor = vec4(texture(Sampler2D, InTexcoord).rrr, 1.0f);
#if 1
	//!< sampler2DShadowとsampler2D : 
	//const float ShadowFactor = InTexcoord.z/InTexcoord.w > texture(Sampler2D, InTexcoord.xy/InTexcoord.w).x ? 0.0f : 1.0f;

	//!< textureProj()とtexture() : textureProj(Sampler2DShadow, InTexcoord) == texture(Sampler2DShadow, InTexcoord.xyz/InTexcoord.w)
	const float ShadowFactor = textureProj(Sampler2DShadow, InTexcoord);
	//const float ShadowFactor = texture(Sampler2DShadow, InTexcoord.xyz/InTexcoord.w);
#else
	const float ShadowFactor = (textureProjOffset(Sampler2DShadow, InTexcoord, ivec2(-1, -1)) + textureProjOffset(Sampler2DShadow, InTexcoord, ivec2(-1,  1)) + textureProjOffset(Sampler2DShadow, InTexcoord, ivec2( 1,  1)) + textureProjOffset(Sampler2DShadow, InTexcoord, ivec2( 1, -1))) * 0.25f;
#endif
	OutColor = vec4(ShadowFactor, ShadowFactor, ShadowFactor, 1.0f);
}