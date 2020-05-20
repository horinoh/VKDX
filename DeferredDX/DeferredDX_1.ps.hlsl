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

float4 main(IN In) : SV_TARGET
{
	//!< UVと深度からワールド位置を求める
	const float2 UVSnorm = float2(In.Texcoord.x, 1.0f - In.Texcoord.y) * 2.0f - 1.0f;
	const float Depth = Texture2.Sample(Sampler, In.Texcoord).r;
	float4 WorldPos = mul(InverseViewProjection, float4(UVSnorm, Depth, 1.0f));
	WorldPos.xyz /= WorldPos.w;
	//return float4(WorldPos.x * 0.5f + 0.5f, 0.0f, 0.0f, 1.0f);
	//return float4(0.0f, WorldPos.y * 0.5f + 0.5f, 0.0f, 1.0f);
	//return float4(0.0f, 0.0f, WorldPos.z * 0.5f + 0.5f, 1.0f);

	return Texture.Sample(Sampler, In.Texcoord);
}
