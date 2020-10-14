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

static const float PI = 4.0f * atan(1.0f);
float Gauss(const float x, const float sigma2)
{
	const float coeff = 1.0f / sqrt(2.0f * PI * sigma2);
	const float expon = -(x * x) / (2.0f * sigma2);
	return coeff * exp(expon);
}
float3 GaussianFilter(const Texture2D textureMap, const int2 coord, const int2 hv)
{
	//!< ウエイトを毎回求めているので効率は悪い
	const int n = 8;
	float weights[n];
	const float sigma2 = 8.0f; //!< 分散
	float sum = 0.0f;
	[unroll]
	for (int k = 0; k < n; ++k) {
		weights[k] = Gauss(k, sigma2);
		sum += sign(k) * 2.0f * weights[k];
	}
	sum = 1.0f / sum;
	[unroll]
	for (int j = 0; j < n; ++j) {
		weights[j] *= sum;
	}

	const int3 coord3 = int3(coord, 0);
	float3 color = textureMap.Load(coord3).rgb * weights[0];
	[unroll]
	for (int i = 1; i < n; ++i) {
		const int2 offset = int2(i, i) * hv;
		color += textureMap.Load(coord3, offset).rgb * weights[i];
		color += textureMap.Load(coord3, -offset).rgb * weights[i];
	}
	return color;
}
float3 GaussianFilterH(const Texture2D textureMap, const int2 coord) { return GaussianFilter(textureMap, coord, int2(1, 0)); }
float3 GaussianFilterV(const Texture2D textureMap, const int2 coord) { return GaussianFilter(textureMap, coord, int2(0, 1)); }

float4 main(IN In) : SV_TARGET
{
#if 0
	return Texture.Sample(Sampler, In.Texcoord);
#elif 0
	//!< セピア (Sepia)
	const float3x3 YCbCr2RGB = { {1.0f, 0.0f, 1.402f}, {1.0f, -0.34414f, -0.71414f}, {1.0f, 1.772f, 0.0f} };
	const float3 YCbCr = float3(dot(float3(0.299f, 0.587f, 0.114f), Texture.Sample(Sampler, In.Texcoord).rgb), -0.2f, 0.1f);
	return float4(mul(YCbCr2RGB, YCbCr), 1.0f);
#elif 0
	//!< モノトーン (Monotone)
	const float Mono = dot(float3(0.299f, 0.587f, 0.114f), Texture.Sample(Sampler, In.Texcoord).rgb);
	return float4(Mono, Mono, Mono, 1.0f);
#elif 0
	//!< ラスター (Raster)
	const float PI = 4.0f * atan(1.0f);
	const float2 UV = In.Texcoord + float2(0.05f * cos(In.Texcoord.y * PI * 10.0f), 0.0f);
	return Texture.Sample(Sampler, UV);
#elif 0
	//!< 輪郭検出 (Edge detection)
	const float2 Center = ToHue(Texture.Sample(Sampler, In.Texcoord).rgb);
	//const float2 Inv = In.Texcoord / max(In.Position.xy, float2(1.0f, 1.0f)); //!< //!< スクリーンサイズ(In.Position.xy / In.Texcoord)の逆数
	int2 Dim; Texture.GetDimensions(Dim.x, Dim.y); const float2 Inv = 1.0f / Dim;
	const float2 Ndx = ToHue(Texture.Sample(Sampler, In.Texcoord + Inv.x * float2(-1.0f, 0.0f)).rgb) - Center;
	const float2 Pdx = ToHue(Texture.Sample(Sampler, In.Texcoord + Inv.x * float2(1.0f, 0.0f)).rgb) - Center;
	const float2 Ndy = ToHue(Texture.Sample(Sampler, In.Texcoord + Inv.y * float2(0.0f, -1.0f)).rgb) - Center;
	const float2 Pdy = ToHue(Texture.Sample(Sampler, In.Texcoord + Inv.y * float2(0.0f, 1.0f)).rgb) - Center;
	float C = dot(Ndx, Ndx) + dot(Pdx, Pdx) + dot(Ndy, Ndy) + dot(Pdy, Pdy);
	return 1.0f - float4(C, C, C, 0.0f);
#elif 0
	//!< 輪郭検出 (Edge detection) ... ddx, ddt
	const float2 Center = ToHue(Texture.Sample(Sampler, In.Texcoord).rgb);
	const float C = length(ddx(Center) + ddy(Center));
	return 1.0f - float4(C, C, C, 0.0f);
#elif 0
	//!< ガウスフィルタ (GaussianFilter) ... 本来は2パス必要
	return float4(GaussianFilterH(Texture, int2(In.Position.xy)), 1.0f);
	//return float4(GaussianFilterV(Texture, int2(In.Position.xy)), 1.0f);
#elif 0
	//!< ディザ (Dither)	
	const float N = 4.0f;
	const float4x4 Bayer = float4x4( 0.0f, 8.0f, 2.0f, 10.0f,
									12.0f, 4.0f, 14.0f, 6.0f,
									 3.0f, 11.0f, 1.0f, 9.0f,
									15.0f, 7.0f, 13.0f, 5.0f);
	int2 TexSize; Texture.GetDimensions(TexSize.x, TexSize.y);
	const int2 BayerUV = int2(fmod(floor(In.Texcoord.x * TexSize.x), N), fmod(floor(In.Texcoord.y * TexSize.y), N));
	const float Threshold = Bayer[BayerUV.x][BayerUV.y] / (N * N);

	const float Mono = dot(float3(0.299f, 0.587f, 0.114f), Texture.Sample(Sampler, In.Texcoord).rgb);
	
	const bool b = Threshold > Mono;
	return float4(b, b, b, 1.0f);
#else
	//!< モザイク (Mosaic)
	const float2 Resolution = float2(800.0f, 600.0f);
	const float Block = 10.0f;
	const float2 UV = floor(In.Texcoord * Resolution / Block) * Block / Resolution;
	return Texture.Sample(Sampler, UV);
#endif
}
