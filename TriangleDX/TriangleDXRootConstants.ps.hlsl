struct IN
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

struct RootConstant
{
	float4 Color;
};
ConstantBuffer<RootConstant> RCInst : register(b0, space0);

float4 main(IN In) : SV_TARGET
{
	return RCInst.Color;
}
