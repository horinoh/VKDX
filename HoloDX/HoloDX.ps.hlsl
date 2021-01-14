struct IN
{
	float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float3 ViewDirection : TEXCOORD0;
    uint Viewport : SV_ViewportArrayIndex;
#if 1
    float ViewIndex : TEXCOORD1;
#endif
};
struct OUT
{
	float4 Color : SV_TARGET;
};

float3 diffuse(const float3 MC, const float3 LC, const float LN)
{
    return saturate(saturate(LN) * MC * LC);
}
float3 specular(const float3 MC, const float4 LC, const float LN, const float3 L, const float3 N, const float3 V)
{
    return saturate(saturate(sign(LN)) * pow(saturate(dot(reflect(-L, N), V)), LC.a) * LC.rgb * MC); // phong
}

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

    //!< N
    const float3 N = normalize(In.Normal);

	//!< L
    const float3 LightDirection = float3(1.0f, 0.0f, 0.0f);
    const float3 L = normalize(LightDirection);

	//!< LN
    const float LN = dot(L, N);

	//!< V
    const float3 V = normalize(In.ViewDirection);

	//!< Color
    const float3 MC = float3(0.5f, 0.5f, 0.5f);
    const float4 LC = float4(1.0f, 1.0f, 1.0f, 32.0f);

    const float3 Amb = float3(0.25f, 0.25f, 0.25f);
    const float3 Dif = diffuse(MC, LC.rgb, LN);
    const float3 Spc = specular(MC, LC, LN, L, N, V);
    const float Atn = 1.0f;
    const float Spt = 1.0f;

    Out.Color = float4((Amb + (Dif + Spc) * Atn) * Spt, 1.0f);

#if 0
    Out.Color.rgb = In.ViewIndex;
#endif
    
	return Out;
}

