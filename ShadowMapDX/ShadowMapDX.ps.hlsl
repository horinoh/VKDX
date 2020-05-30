struct IN
{
	float4 Position : SV_POSITION;
	//float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	Out.Color = float4(In.Depth.x / In.Depth.y, 0.0f, 0.0f, 1.0f);
	return Out;
}

