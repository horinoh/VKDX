#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

//!< サンプラ
//layout (set=0, binding=0) uniform sampler Sampler;
//!< サンプルドイメージ
//layout (set=0, binding=0) uniform texture2D Texture;

//!< コンバインドイメージサンプラ(サンプラ + サンプルドイメージ) ... アプリからは通常通りサンプラとサンプルドイメージを作成する、シェーダ内での解釈のみが異なる
layout (set=0, binding=0) uniform sampler2D Sampler2D;

void main()
{
	//OutColor = vec4(InTexcoord, 0.0f, 1.0f);
	OutColor = texture(Sampler2D, InTexcoord);
}