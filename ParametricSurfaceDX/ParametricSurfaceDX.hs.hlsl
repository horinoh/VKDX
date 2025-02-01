struct IN
{
	float3 Position : POSITION;
}; 
struct OUT
{
	float3 Position : POSITION;
};

struct TESS_FACTOR
{
	float Edge[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};
TESS_FACTOR ConstantHS(/*const InputPatch<IN, 1> patch, const uint p : SV_PrimitiveID*/)
{
	TESS_FACTOR Out;
	const float Edge = 15.0f;
	const float Inside = 15.0f;
	Out.Edge[0] = Edge;
	Out.Edge[1] = Edge;
	Out.Edge[2] = Edge;
	Out.Edge[3] = Edge;
	Out.Inside[0] = Inside;
	Out.Inside[1] = Inside;
	return Out;
}

[domain("quad")] // tri, quad, isoline
[partitioning("integer")] // fractional_odd, fractional_even, integer, pow2
[outputtopology("triangle_cw")] // point, line, triangle_cw, triangle_ccw
[outputcontrolpoints(4)] // 出力コントロールポイント数
[patchconstantfunc("ConstantHS")] // パッチ関数
[maxtessfactor(64.0f)]
OUT main(/*const InputPatch<IN, 1> patch, const uint i : SV_OutputControlPointID, const uint p : SV_PrimitiveID*/)
{
	return (OUT)0;
}
