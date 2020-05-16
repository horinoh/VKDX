struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);

float2 ToHue(const float3 Color)
{
	return float2(dot(float3(-0.1687f, -0.3312f, 0.5f), Color), dot(float3(0.5f, -0.4183f, -0.0816f), Color));
}

float4 main(IN In) : SV_TARGET
{
#if 0
	return Texture.Sample(Sampler, In.Texcoord);
#elif 0
	//!< セピア
	float3x3 YCbCr2RGB = { {1.0f, 0.0f, 1.402f}, {1.0f, -0.34414f, -0.71414f}, {1.0f, 1.772f, 0.0f} };
	float3 YCbCr = float3(dot(float3(0.299f, 0.587f, 0.114f), Texture.Sample(Sampler, In.Texcoord).rgb), -0.2f, 0.1f);
	return float4(mul(YCbCr2RGB, YCbCr), 1.0f);
#elif 0
	//!< モノトーン
	float Mono = dot(float3(0.299f, 0.587f, 0.114f), Texture.Sample(Sampler, In.Texcoord).rgb);
	return float4(Mono, Mono, Mono, 1.0f);
#elif 0
	//!< ラスター
	const float PI = 4.0f * atan(1.0f);
	const float2 UV = In.Texcoord + float2(0.05f * cos(In.Texcoord.y * PI * 10.0f), 0.0f);
	return Texture.Sample(Sampler, UV);
#elif 0
	//!< 輪郭検出
	const float2 Center = ToHue(Texture.Sample(Sampler, In.Texcoord).rgb);
	const float2 Ndx = ToHue(Texture.Sample(Sampler, In.Texcoord + 0.001f * float2(-1.0f, 0.0f)).rgb) - Center;
	const float2 Pdx = ToHue(Texture.Sample(Sampler, In.Texcoord + 0.001f * float2(1.0f, 0.0f)).rgb) - Center;
	const float2 Ndy = ToHue(Texture.Sample(Sampler, In.Texcoord + 0.001f * float2(0.0f, -1.0f)).rgb) - Center;
	const float2 Pdy = ToHue(Texture.Sample(Sampler, In.Texcoord + 0.001f * float2(0.0f, 1.0f)).rgb) - Center;
	float C = dot(Ndx, Ndx) + dot(Pdx, Pdx) + dot(Ndy, Ndy) + dot(Pdy, Pdy);
	return 1.0f - float4(C, C, C, 1.0f);
#else
	//!< モザイク
	const float2 Resolution = float2(800.0f, 600.0f);
	const float Block = 10.0f;
	const float2 UV = floor(In.Texcoord * Resolution / Block) * Block / Resolution;
	return Texture.Sample(Sampler, UV);
#endif
}
