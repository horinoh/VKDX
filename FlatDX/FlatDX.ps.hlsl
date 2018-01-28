struct IN
{
	float4 Position : SV_POSITION;
	nointerpolation float3 Normal : NORMAL;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = float4(In.Normal * 0.5f + 0.5f, 1.0f);
	return Out;
}

