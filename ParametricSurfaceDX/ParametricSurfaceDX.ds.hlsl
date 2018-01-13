struct IN
{
	float3 Position : POSITION;
}; 
struct TESS_FACTOR
{
	float Edge[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};
struct OUT
{
	float3 Position : POSITION;
};

static const float PI = 4.0f * atan(1.0f);
float2 GetUV_Torus(const float2 uv)
{
	return frac(uv) * 2.0f * PI;
}
float3 GetPosition_Torus(const float2 uv)
{
	const float2 UV = GetUV_Torus(uv);
	const float2 R = float2(0.5f, 1.0f);
	return float3((R.y + R.x * cos(UV.y)) * cos(UV.x), (R.y + R.x * cos(UV.y)) * sin(UV.x), R.x * sin(UV.y));
}

[domain("quad")]
OUT main(const TESS_FACTOR tess, const float2 uv : SV_DomainLocation, const OutputPatch<IN, 4> quad)
{
	OUT Out;
#if 1
	Out.Position = GetPosition_Torus(uv) * 0.5f;
#else
	Out.Position = float3(2.0f * uv - 1.0f, 1.0f);
#endif
	return Out;
}
