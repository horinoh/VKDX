struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; float4x4 InverseViewProjection; };

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);	//!< カラー(Color)
Texture2D Texture1 : register(t1, space0);	//!< 法線(Normal)
Texture2D Texture2 : register(t2, space0);	//!< 深度(Depth)
Texture2D Texture3 : register(t3, space0);	//!< 未定

float Random(float2 uv, int seed) { return frac(sin(dot(uv, float2(12.9898f, 78.233f)) + seed) * 43758.5453f); }
static const float PI = 4.0f * atan(1.0f);
float CosLerp(float lhs, float rhs, float t) { return lerp(lhs, rhs, 0.5f * (1.0f - cos(PI * t))); }
float Hermite3Lerp(float lhs, float rhs, float t) { return lerp(lhs, rhs, pow(t, 2.0f) * (3.0f - 2.0f * t)); }
float Hermite5Lerp(float lhs, float rhs, float t) { return lerp(lhs, rhs, pow(t, 3.0f) * (6.0f * pow(t, 2.0f) - 15.0f * t + 10.0f)); }
float PerlinNoise(float2 uv, float freq, float2 dxy)
{
	float2 x = freq * uv;
	float2 i = floor(x) * dxy;
	float2 f = frac(x);
	int seed = 0;
#if 1
	return lerp(
		lerp(Random(i, seed), Random(i + float2(dxy.y, 0.0f), seed), f.x),
		lerp(Random(i + float2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return CosLerp(
		CosLerp(Random(i, seed), Random(i + float2(dxy.y, 0.0f), seed), f.x),
		CosLerp(Random(i + float2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return Hermite3Lerp(
		Hermite3Lerp(Random(i, seed), Random(i + float2(dxy.y, 0.0f), seed), f.x),
		Hermite3Lerp(Random(i + float2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#elif 0
	return Hermite5Lerp(
		Hermite5Lerp(Random(i, seed), Random(i + float2(dxy.y, 0.0f), seed), f.x),
		Hermite5Lerp(Random(i + float2(0.0f, dxy.y), seed), Random(i + dxy, seed), f.x),
		f.y);
#endif
}

float4 main(IN In) : SV_TARGET
{
	//!< UVと深度からワールド位置を求める
	const float Depth = Texture2.Sample(Sampler, In.Texcoord).r;
	const float2 UVSnorm = float2(In.Texcoord.x, 1.0f - In.Texcoord.y) * 2.0f - 1.0f;
	float4 WorldPos = mul(InverseViewProjection, float4(UVSnorm, Depth, 1.0f));
	WorldPos.xyz /= WorldPos.w;
	//return float4(WorldPos.x * 0.5f + 0.5f, 0.0f, 0.0f, 1.0f);
	//return float4(0.0f, WorldPos.y * 0.5f + 0.5f, 0.0f, 1.0f);
	//return float4(0.0f, 0.0f, WorldPos.z * 0.5f + 0.5f, 1.0f);

	const float3 N = normalize(Texture1.Sample(Sampler, In.Texcoord).xyz * 2.0f - 1.0f);

	const float3 LightPos = float3(0.0f, 10.0f, 0.0f);
	const float3 L = normalize(LightPos - WorldPos.xyz);

	//const float Amplitude = 1.0f;
	//const float Frequency = 500.0f;
	//const float n = Amplitude * PerlinNoise(In.Texcoord, Frequency, float2(0.01f, 0.01f));
	//return float4(n, n, n, 1.0f);

	if (Depth < 0.99f) {
		return Texture.Sample(Sampler, In.Texcoord) * saturate(dot(N, L));
	}
	else {
		return Texture.Sample(Sampler, In.Texcoord);
	}
}
