#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTexcoord;
layout (location = 1) in vec3 InTangentViewDirection;
layout (location = 2) in vec3 InTangentLightDirection;

//!< セパレートサンプラ : texture(sampler2D(texture2D, sampler), TexCoord)
layout(set = 0, binding = 1) uniform sampler Sampler;
layout(set = 0, binding = 2) uniform texture2D NormalMap;
layout(set = 0, binding = 3) uniform texture2D ColorMap;

layout (location = 0) out vec4 Color;

vec3 diffuse(const vec3 MC, const vec3 LC, const float LN) 
{ 
	return clamp(clamp(LN, 0.0f, 1.0f) * MC * LC, 0.0f, 1.0f); 
}
vec3 specular(const vec3 MC, const vec4 LC, const float LN, const vec3 L, const vec3 N, const vec3 V)
{
	return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(reflect(-L, N), V), 0.0f, 1.0f), LC.a) * LC.rgb * MC, 0.0f, 1.0f); // phong
	//return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(N, normalize(V + L)), 0.0f, 1.0f), LC.a) * LC.rgb * MC, 0.0f, 1.0f); // blinn
}

layout (early_fragment_tests) in;
void main()
{
	//!< V : ピクセルからカメラへ向かう方向
	const vec3 V = normalize(InTangentViewDirection);
	
	//!< N
	const vec3 N = texture(sampler2D(NormalMap, Sampler), InTexcoord).xyz * 2.0f - 1.0f;

	//!< L : ピクセルからライトへ向かう方向
	const vec3 L = normalize(InTangentLightDirection);

	//!< LN
	const float LN = dot(L, N);

	//!< Color
	const vec3 MC = texture(sampler2D(ColorMap, Sampler), InTexcoord).rgb;
	const vec4 LC = vec4(1.0f, 1.0f, 1.0f, 32.0f);

	const vec3 Amb = vec3(0.25f, 0.25f, 0.25f); // * texture(sampler2D(AOMap, Sampler), InTexcoord).rgb;
	const vec3 Dif = diffuse(MC, LC.rgb, LN);
	const vec3 Spc = specular(MC, LC, LN, L, N, V);
	const float Atn = 1.0f; //pow(max(1.0f - 1.0f / LightRange * LightLen, 0.0f), 2.0f);
	const float Spt = 1.0f; //pow(max(1.0f - 1.0f / SpotRange * abs(acos(dot(L, SpotDir))), 1.0f), 0.5f);
	const vec3 Rim = vec3(0.0f); //pow(1.0f - max(-V.z, 0.0f), 1.3f) * LC.rgb;
	//const vec3 Rim = vec3(0.0f); //pow((1.0f - max(dot(L, n), 0.0f)) * (1.0f - max(-V.z, 0.0f)), 1.3f) * LC.rgb;
	const vec3 Hemi = vec3(0.0f);//mix(GrdCol, SkyCol, dot(n, vec3(0.0f, 1.0f, 0.0f)) * 0.5f + 0.5f);

	Color = vec4(Amb + (Dif + Spc) * Atn * Spt + Rim + Hemi, 1.0f);

	//Color = vec4(InTexcoord, 0.0f, 1.0f);
	//Color = vec4(texture(NormalMap, InTexcoord).rgb, 1.0f);
}