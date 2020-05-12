#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

//!< アプリからは「コンバインドイメージサンプラ」(サンプラ + サンプルドイメージ)としてバインディング(0)で作成している為
//!< 「コンバインドイメージサンプラ」として使う場合も、「サンプラ+サンプルドイメージ」として使う場合でも同じバインディング(0)にバインドされている
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