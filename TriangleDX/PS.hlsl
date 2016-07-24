struct IN
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

#if 0
struct CB
{
	float4 Color;
};
ConstantBuffer<CB> CBInst : register(b0);
//cbuffer CB : register(b0)
//{
//	float4 Color;
//}
#endif

float4 main(IN In) : SV_TARGET
{
#if 0
	return CBInst.Color;
#else
	return In.Color;
#endif
}
