struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
}; 

static const float Scale = 1.0f;

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;

	return Out;
}
