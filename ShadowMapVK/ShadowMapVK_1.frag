#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

//layout (location = 0) in vec2 InTexcoord;
layout (location = 0) in vec4 InTexcoord;

layout (location = 0) out vec4 OutColor;

//layout (set=0, binding=0) uniform sampler2D Sampler2D;
layout (set=0, binding=0) uniform sampler2DShadow Sampler2D;

void main()
{
	//OutColor = vec4(texture(Sampler2D, InTexcoord).rrr, 1.0f);
#if 0
	const float ShadowFactor = textureProj(Sampler2D, InTexcoord);
#else
	const float ShadowFactor = (textureProjOffset(Sampler2D, InTexcoord, ivec2(-1, -1)) + textureProjOffset(Sampler2D, InTexcoord, ivec2(-1,  1)) + textureProjOffset(Sampler2D, InTexcoord, ivec2( 1,  1)) + textureProjOffset(Sampler2D, InTexcoord, ivec2( 1, -1))) * 0.25f;
#endif
	OutColor = vec4(ShadowFactor, ShadowFactor, ShadowFactor, 1.0f);
}