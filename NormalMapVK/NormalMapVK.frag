#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal;
layout (location = 1) in vec4 InTangent;
layout (location = 2) in vec2 InTexcoord;

//layout (binding = 0) uniform sampler2D NormalMap;

layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	const vec3 N = normalize(InNormal);
	const vec3 T = normalize(InTangent.xyz - dot(InTangent.xyz, N) * N);
	const vec3 B = cross(N, T) * InTangent.w;
	const mat3 TBN = mat3(T, B, N);

	//const vec3 Normal = TBN * (texture(NormalMap, InTexcoord).xyz * 2.0f - 1.0f);	
	//Color = vec4(Normal * 0.5f + 0.5f, 1.0f);

	//Color = vec4(InNormal * 0.5f + 0.5f, 1.0f);
	Color = vec4(InTangent.xyz * 0.5f + 0.5f, 1.0f);
}