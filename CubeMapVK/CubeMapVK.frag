#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTexcoord;
layout (location = 1) in vec3 InViewDirection;

layout (set = 0, binding = 1) uniform samplerCube CubeMap;
layout (set = 0, binding = 2) uniform sampler2D NormalMap;

layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	//!< V : �s�N�Z������J�����֌���������
	const vec3 V = normalize(InViewDirection);
	
	//!< N
	const vec3 N = texture(NormalMap, InTexcoord).xyz * 2.0f - 1.0f;

	const vec3 Reflection = texture(CubeMap, reflect(-V, N)).rgb;

	//!< ���ܗ� (��:1.33f, �O���X:1.52f, ��C:1.00029f, �^��:1.0f)
	const float RefractionIndex = 1.00029f / 1.33f;
	const vec3 Refraction = texture(CubeMap, refract(-V, N, RefractionIndex)).rgb;

	Color = vec4(mix(Reflection, Refraction, clamp(dot(V, N), 0.0f, 1.0f)), 1.0f);
	//Color = vec4(Reflection, 1.0f);
	//Color = vec4(Refraction, 1.0f);
}