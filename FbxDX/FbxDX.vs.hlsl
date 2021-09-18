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
	const float4x4 WVP = transpose(float4x4(1.93643105f, 0.0f, 0.0f, 0.0f, 
											0.0f, 3.89474249f, 0.0f, 0.0f,
											0.0f, 0.0f, -1.00010002f, -1.0f,
											0.0f, 0.0f, 2.99029899f, 3.0f));
	Out.Position = mul(WVP, float4(In.Position, 1.0f));
	Out.Color = In.Normal * 0.5f + 0.5f;

	return Out;
}
