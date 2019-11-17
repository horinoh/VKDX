struct IN
{
	float3 Tangent : TANGENT;
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct OUT
{
	float3 Tangent : TANGENT;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

static const float Scale = 0.5f;

OUT main(IN In)
{
	OUT Out;

	Out.Tangent = In.Tangent;
	Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;
	Out.Texcoord = In.Texcoord;

	return Out;
}
