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

Texture2D ColorMap : register(t1, space0);
SamplerState Sampler : register(s1, space0);

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = ColorMap.Sample(Sampler, In.Texcoord);
	//Out.Color =float4(In.Texcoord, 0,1);
	return Out;
}

