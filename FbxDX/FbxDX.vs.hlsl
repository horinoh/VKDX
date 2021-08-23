struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position, 1.0f);
	Out.Color = In.Normal * 0.5f + 0.5f;
	Out.Color = Out.Position.zzz;

	return Out;
}
