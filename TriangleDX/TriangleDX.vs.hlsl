struct IN
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
}; 

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position, 1.0f);
	Out.Color = In.Color;

	return Out;
}
