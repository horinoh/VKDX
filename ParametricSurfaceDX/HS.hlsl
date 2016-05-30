struct IN
{
	float3 Dummy : POSITION;
}; 
struct TESS_FACTOR
{
	float Edge[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};
struct OUT
{
	float3 Dummy : POSITION;
};

TESS_FACTOR chs()
{
	TESS_FACTOR Out;
	const float t = 15;
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
[patchconstantfunc("chs")]
[maxtessfactor(64.0f)]
OUT main(const InputPatch<IN, 4> patch, uint i : SV_OutputControlPointID, uint p : SV_PrimitiveID)
{
	return (OUT)0;
}
