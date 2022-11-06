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
	float2 Texcoord : TEXCOORD0;
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
float2 GetUV_Sphere(const float2 uv)
{
	return (frac(uv) * float2(1.0f, -1.0f) + float2(0.0f, 0.5f)) * 2.0f * PI;
}
float3 GetPosition_Sphere(const float2 uv)
{
	const float2 UV = GetUV_Sphere(uv);
	const float3 R = float3(1.0f, 1.0f, 1.0f);
    return 2.0f * R * float3(cos(UV.x) * sin(UV.y), sin(UV.x) * sin(UV.y), cos(UV.y));
}
float2 GetUV_Snail(const float2 uv)
{
	return (frac(uv) * float2(1.0f, -1.0f) + float2(0.0f, 0.5f)) * 2.0f * PI;
}
float3 GetPosition_Snail(const float2 uv)
{
	const float2 UV = GetUV_Snail(uv);
	return UV.xxx * float3(cos(UV.y) * sin(UV.x), cos(UV.x) * cos(UV.y), -sin(UV.y));
}
float2 GetUV_Pillow(const float2 uv)
{
	return (frac(uv) * float2(1.0f, -1.0f) + float2(0.0f, 1.0f)) * 2.0f * PI;
}
float3 GetPosition_Pillow(const float2 uv)
{
	const float2 UV = GetUV_Pillow(uv);
	return float3(cos(UV.x), cos(UV.y), 0.5f * sin(UV.x) * sin(UV.y));
}

[domain("quad")]
OUT main(const TESS_FACTOR tess, const float2 uv : SV_DomainLocation, const OutputPatch<IN, 4> quad)
{
	OUT Out;
	Out.Position = GetPosition_Torus(uv) * 0.5f;
	//Out.Position = GetPosition_Sphere(uv) * 0.5f;
	//Out.Position = GetPosition_Snail(uv) * 0.1f;
	//Out.Position = GetPosition_Pillow(uv) * 0.5f;
	//Out.Position = float3(2.0f * uv - 1.0f, 1.0f);

	Out.Texcoord = float2(uv.x, 1.0f - uv.y);
	return Out;
}
