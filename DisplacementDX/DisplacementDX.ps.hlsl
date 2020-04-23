struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

//Texture2D NormalMap : register(t0, space0);
//SamplerState Sampler : register(s0, space0);

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return Out;
}

