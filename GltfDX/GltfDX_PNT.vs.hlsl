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

struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

OUT main(IN In)
{
	OUT Out;

    const float4x4 PV = mul(Transform.Projection, Transform.View);
	//const float4x4 PV = mul(Transform.View, Transform.Projection);

	Out.Position = mul(PV, float4(In.Position, 1.0f));
	//Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;
	Out.Texcoord = In.Texcoord;

	return Out;
}
