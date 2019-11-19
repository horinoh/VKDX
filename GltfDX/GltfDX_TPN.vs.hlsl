struct IN
{
	float3 Tangent : TANGENT;
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

struct OUT
{
	float3 Tangent : TANGENT;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

static const float Scale = 50.0f;

OUT main(IN In)
{
	OUT Out;

	Out.Tangent = In.Tangent;
	Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;

	return Out;
}
