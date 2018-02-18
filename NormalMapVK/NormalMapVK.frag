#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal;
layout (location = 1) in vec4 InTangent;
layout (location = 2) in vec2 InTexcoord;
layout (location = 3) in vec3 InViewDirection;

layout (binding = 0) uniform sampler2D NormalMap;

layout (location = 0) out vec4 Color;

vec3 diffuse(const vec3 MC, const vec3 LC, const float LN) { return clamp(clamp(LN, 0.0f, 1.0f) * MC * LC, 0.0f, 1.0f); }
vec3 specular(const vec3 MC, const vec4 LC, const float LN, const vec3 L, const vec3 N, const vec3 V)
{
	return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(reflect(-L, N), V), 0.0f, 1.0f), LC.a) * LC.rgb * MC, 0.0f, 1.0f); // phong
	//return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(N, normalize(V + L)), 0.0f, 1.0f), LC.a) * LC.rgb * MC, 0.0f, 1.0f); // blinn
}

layout (early_fragment_tests) in;
void main()
{
	//!< N
	const vec3 n = normalize(InNormal);
	const vec3 t = normalize(InTangent.xyz - dot(InTangent.xyz, n) * n);
	const vec3 b = cross(n, t) * InTangent.w;
	const mat3 tbn = mat3(t, b, n);
	const vec3 N = n;
	//const vec3 N = tbn * (texture(NormalMap, InTexcoord).xyz * 2.0f - 1.0f); // TODO

	//!< L
	const vec3 LightDirection = vec3(0, 1, 0); // TODO
	const vec3 L = normalize(LightDirection);

	//!< LN
	const float LN = dot(L, N);

	//!< V
	const vec3 V = normalize(InViewDirection);

	//!< Color
	const vec3 MC = vec3(0.5f, 0.5f, 0.5f);
	const vec4 LC = vec4(1.0f, 1.0f, 1.0f, 32.0f);

	const vec3 Amb = vec3(0.25f, 0.25f, 0.25f);
	const vec3 Dif = diffuse(MC, LC.rgb, LN);
	const vec3 Spc = specular(MC, LC, LN, L, N, V);
	const float Atn = 1.0f;
	const float Spt = 1.0f;

	Color = vec4((Amb + (Dif + Spc) * Atn) * Spt, 1.0f);

	//Color = vec4(n * 0.5f + 0.5f, 1.0f); // revY
	//Color = vec4(t * 0.5f + 0.5f, 1.0f); // revY
	//Color = vec4(b * 0.5f + 0.5f, 1.0f); // revY
	//Color = vec4(InTexcoord, 0.0f, 1.0f); // rev
}