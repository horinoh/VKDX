struct IN
{
	float3 Position : POSITION;
	//float3 Normal : NORMAL;
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
	Out.Color = float3(0, 1, 0) * 0.5f + 0.5f;

	return Out;
}