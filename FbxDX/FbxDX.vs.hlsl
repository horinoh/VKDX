struct IN
{
	float3 Position : POSITION;
	//float3 Normal : NORMAL;
};

struct OUT
{
	float4 Position : SV_POSITION;
}; 

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position, 1.0f);

	return Out;
}
