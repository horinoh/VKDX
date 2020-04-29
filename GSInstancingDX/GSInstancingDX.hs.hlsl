struct IN
{
	float3 Position : POSITION;
	uint Viewport : SV_ViewportArrayIndex;
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

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
OUT main(/*const InputPatch<IN, 1> patch, const uint i : SV_OutputControlPointID, const uint p : SV_PrimitiveID*/)
{
	return (OUT)0;
}
