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
TESS_FACTOR ConstantHS()
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
[outputtopology("triangle_cw")] //!< D3D12_RASTERIZER_DESC.FrontCounterClockwise == TRUE ‚Æ‚©‚¿‡‚¤‚Ì‚Å‚±‚±‚Å‚Í cw ‚ðŽw’è
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
OUT main()
{
	return (OUT)0;
}
