#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (set = 0, binding = 4) uniform Transform { mat4 Projection; mat4 View; mat4 World; mat4 InverseViewProjection; };

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;	//!< カラー(Color)
layout (set=0, binding=1) uniform sampler2D Sampler2D1; //!< 法線(Normal)
layout (set=0, binding=2) uniform sampler2D Sampler2D2;	//!< 深度(Depth)
layout (set=0, binding=3) uniform sampler2D Sampler2D3; //!< 未定

float Random(vec2 uv, int seed) { return fract(sin(dot(uv, vec2(12.9898f, 78.233f)) + seed) * 43758.5453f); }
const float PI = 4.0f * atan(1.0f);
float CosLerp(float lhs, float rhs, float t) { return mix(lhs, rhs, 0.5f * (1.0f - cos(PI * t))); }
float Hermite3Lerp(float lhs, float rhs, float t) { return mix(lhs, rhs, pow(t, 2.0f) * (3.0f - 2.0f * t)); }
float Hermite5Lerp(float lhs, float rhs, float t) { return mix(lhs, rhs, pow(t, 3.0f) * (6.0f * pow(t, 2.0f) - 15.0f * t + 10.0f)); }
float PerlinNoise(vec2 uv, float freq, vec2 dxy)
{
	vec2 x = freq * uv;
	vec2 i = floor(x) * dxy;
	vec2 f = fract(x);
	int seed = 0;
#if 1
	return mix(
		mix(Random(i, seed), Random(i + vec2(dxy.y, 0.0f), seed), f.x),
		mix(Random(i + vec2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return CosLerp(
		CosLerp(Random(i, seed), Random(i + vec2(dxy.y, 0.0f), seed), f.x),
		CosLerp(Random(i + vec2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return Hermite3Lerp(
		Hermite3Lerp(Random(i, seed), Random(i + vec2(dxy.y, 0.0f), seed), f.x),
		Hermite3Lerp(Random(i + vec2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return Hermite5Lerp(
		Hermite5Lerp(Random(i, seed), Random(i + vec2(dxy.y, 0.0f), seed), f.x),
		Hermite5Lerp(Random(i + vec2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#endif
}

void main()
{
	//!< UVと深度からワールド位置を求める
	const float Depth = texture(Sampler2D2, InTexcoord).r;
	const vec2 UVSnorm = vec2(InTexcoord.x, 1.0f - InTexcoord.y) * 2.0f - 1.0f;
	vec4 WorldPos = InverseViewProjection * vec4(UVSnorm, Depth, 1.0f);
	WorldPos.xyz /= WorldPos.w;
	//OutColor = vec4(WorldPos.x * 0.5f + 0.5f, 0.0f, 0.0f, 1.0f);
	//OutColor = vec4(0.0f, WorldPos.y * 0.5f + 0.5f, 0.0f, 1.0f);
	//OutColor = vec4(0.0f, 0.0f, WorldPos.z * 0.5f + 0.5f, 1.0f);

	const vec3 N = normalize(texture(Sampler2D1, InTexcoord).xyz * 2.0f - 1.0f);

	const vec3 LightPos = vec3(0.0f, 10.0f, 0.0f);
	const vec3 L = normalize(LightPos - WorldPos.xyz);

	if (Depth < 0.99f) {
		OutColor = texture(Sampler2D, InTexcoord) * clamp(dot(L, N), 0.0f, 1.0f);
	}
	else{
		OutColor = texture(Sampler2D, InTexcoord);
	}
	
	//const float Amplitude = 1.0f;
	//const float Frequency = 500.0f;
	//const float p = Amplitude * PerlinNoise(InTexcoord, Frequency, vec2(0.01f, 0.01f));
	//OutColor = vec4(p, p, p, 1.0f);
}