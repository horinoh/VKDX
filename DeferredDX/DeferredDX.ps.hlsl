struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Color : SV_TARGET;
//	float4 Color1 : SV_TARGET1;
//	float4 Color2 : SV_TARGET2;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = float4(In.Texcoord, 0.0f, 1.0f);
	return Out;
}

