struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
}; 

static const float Scale = 0.005f;

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;
	Out.Texcoord = In.Texcoord;

	return Out;
}
