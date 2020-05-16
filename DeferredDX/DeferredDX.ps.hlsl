struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};
struct OUT
{
	float4 Color : SV_TARGET;
//	float4 Color1 : SV_TARGET1;
//	float4 Color2 : SV_TARGET2;
//	float4 Color3 : SV_TARGET3;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	Out.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	Out.Color/*1*/ = float4(normalize(In.Normal) * 0.5f + 0.5f, 1.0f);
	Out.Color/*2*/ = float4(In.Depth.x / In.Depth.y, 0.0f, 0.0f, 1.0f);
	//Out.Color/*3*/ = float4(1.0f, 0.0f, 0.0f, 1.0f);

	return Out;
}

