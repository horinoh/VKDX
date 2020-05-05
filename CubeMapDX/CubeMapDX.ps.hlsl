struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
	float3 Position3D : TEXCOORD2;
};

SamplerState Sampler : register(s0, space0);
TextureCube CubeMap : register(t0, space0);
Texture2D NormalMap : register(t1, space0);

struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	//!< V : ピクセルからカメラへ向かう方向
	const float3 V = normalize(In.ViewDirection); 

	//!< N	
	const float3 N = NormalMap.Sample(Sampler, In.Texcoord).xyz * 2.0f - 1.0f;

	const float3 Reflection = CubeMap.Sample(Sampler, reflect(-V, N)).rgb;

	//!< 屈折率 (水:1.33f, グラス:1.52f, 空気:1.00029f, 真空:1.0f)
	const float RefractionIndex = 1.00029f / 1.33f;
	const float3 Refraction = CubeMap.Sample(Sampler, refract(-V, N, RefractionIndex)).rgb;

	Out.Color = float4(lerp(Reflection, Refraction, saturate(dot(V, N))), 1.0f);
	//Out.Color = float4(Reflection, 1.0f);
	//Out.Color = float4(Refraction, 1.0f);

	return Out;
}

