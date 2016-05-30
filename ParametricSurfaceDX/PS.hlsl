struct IN
{
	float4 Position : SV_POSITION;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	return Out;
}

