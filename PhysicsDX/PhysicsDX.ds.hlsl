struct IN
{
    uint InstanceID : SV_InstanceID;
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
    float2 Texcoord : TEXCOORD0;
    uint InstanceID : SV_InstanceID;
};

static const float PI = 4.0f * atan(1.0f);

float2 GetUV_Sphere(const float2 uv)
{
	//!< [0, 2PI] [PI, -PI]
	return (frac(uv) * float2(1.0f, -1.0f) + float2(0.0f, 0.5f)) * 2.0f * PI;
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

float2 GetUV_Cube(const float2 uv)
{
    //return uv * 2.0f - 1.0f;
    return uv * PI * float2(2, 1);
}
float3 GetPosition_Cube(const float2 uv)
{
    const float2 UV = GetUV_Cube(uv);
    const float3 Extent = float3(2.0f, 2.0f, 2.0f);
    const float a = sqrt(sqrt(sqrt(pow(sin(UV.x), 6.0f) * (pow(sin(UV.y), 6.0f) + pow(cos(UV.y), 6.0f)) + pow(cos(UV.x), 6.0f))));
    return Extent * float3(sin(UV.x) * cos(UV.y), sin(UV.x) * sin(UV.y), cos(UV.x)) / a;
}
float3 GetNormal_Cube(const float2 uv, const float3 pos)
{
    const float2 du = float2(0.01f, 0.0f);
    const float2 dv = float2(0.0f, 0.01f);
    return normalize(cross(GetPosition_Cube(uv + du) - pos, GetPosition_Cube(uv + dv) - pos));
}

[domain("quad")]
OUT main(const TESS_FACTOR tess, const float2 uv : SV_DomainLocation, const OutputPatch<IN, 4> quad)
{
	OUT Out;
    
	Out.Position = GetPosition_Sphere(uv) * 0.5f;
    Out.Normal = GetNormal_Sphere(uv, Out.Position);
    
    //Out.Position = GetPosition_Cube(uv) * 0.5f;
    //Out.Normal = GetNormal_Cube(uv, Out.Position);
    
    Out.Texcoord = float2(uv.x, 1.0f - uv.y);
    Out.InstanceID = quad[0].InstanceID;
	return Out;
}
