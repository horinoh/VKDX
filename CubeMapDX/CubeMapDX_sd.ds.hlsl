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

float2 GetUV_Sphere(const float2 uv)
{
	return frac(uv) * float2(2.0f, 1.0f) * PI;
}
float3 GetPosition_Sphere(const float2 uv)
{
	const float2 UV = GetUV_Sphere(uv);
	const float3 R = float3(1.0f, 1.0f, 1.0f);
	return R * float3(cos(UV.x) * sin(UV.y), sin(UV.x) * sin(UV.y), cos(UV.y));
}

[domain("quad")]
OUT main(const TESS_FACTOR tess, const float2 uv : SV_DomainLocation, const OutputPatch<IN, 4> quad)
{
	OUT Out;

	Out.Position = GetPosition_Sphere(uv) * 0.5f;

	return Out;
}
