struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
}; 

static const float Scale = 0.005f;
//static const float Scale = 0.5f;
//static const float Scale = 1.0f;

cbuffer ProjView : register(b0, space0) { float4x4 Projection; float4x4 View; };

OUT main(IN In)
{
	OUT Out;

	const float4x4 PV = mul(Projection, View);
	//const float4x4 PV = mul(View, Projection);

	Out.Position = mul(PV, float4(In.Position, 1.0f));
	//Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;
	Out.Texcoord = In.Texcoord;

	return Out;
}
