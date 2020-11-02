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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
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
float3 GetNormal_Torus(const float2 uv, const float3 pos)
{
	const float2 du = float2(0.01f, 0.0f);
	const float2 dv = float2(0.0f, 0.01f);
	return normalize(cross(GetPosition_Torus(uv + du) - pos, GetPosition_Torus(uv + dv) - pos));
}
float3 GetTangent_Torus(const float2 uv, const float3 pos)
{
	const float2 du = float2(0.01f, 0.0f);
	return normalize(GetPosition_Torus(uv + du) - pos);
}
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
float3 GetNormal_Sphere(const float2 uv, const float3 pos)
{
	const float2 du = float2(0.01f, 0.0f);
	const float2 dv = float2(0.0f, 0.01f);
	return normalize(cross(GetPosition_Sphere(uv + du) - pos, GetPosition_Sphere(uv + dv) - pos));
}
float3 GetTangent_Sphere(const float2 uv, const float3 pos)
{
	const float2 du = float2(0.01f, 0.0f);
	return normalize(GetPosition_Sphere(uv + du) - pos);
}

[domain("quad")]
OUT main(const TESS_FACTOR tess, const float2 uv : SV_DomainLocation, const OutputPatch<IN, 4> quad)
{
	OUT Out;

	Out.Texcoord = float2(uv.x, 1.0f - uv.y);
#if 1
	Out.Position = GetPosition_Torus(uv) * 0.5f;
	Out.Normal = GetNormal_Torus(uv, Out.Position);
	Out.Tangent = GetTangent_Torus(uv, Out.Position);
#elif 1
	Out.Position = GetPosition_Sphere(uv) * 0.5f;
	Out.Normal = GetNormal_Sphere(uv, Out.Position);
	Out.Tangent = GetTangent_Sphere(uv, Out.Position);
#else
	Out.Position = float3(2.0f * uv - 1.0f, 0.0f);
	Out.Normal = float3(0.0f, 1.0f, 0.0f);
	Out.Tangent = float3(1.0f, 0.0f, 0.0f);
#endif
	return Out;
}
