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
	const float t = 15.0f;
	Out.Edge[0] = t;
	Out.Edge[1] = t;
	Out.Edge[2] = t;
	Out.Edge[3] = t;
	Out.Inside[0] = t;
	Out.Inside[1] = t;
	return Out;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
OUT main(/*const InputPatch<IN, 1> patch, const uint i : SV_OutputControlPointID, const uint p : SV_PrimitiveID*/)
{
	return (OUT)0;
}
