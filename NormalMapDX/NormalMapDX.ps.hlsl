struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 TangentViewDirection : TEXCOORD1;
	float3 TangentLightDirection : TEXCOORD2;
};

Texture2D NormalMap : register(t0, space0);
Texture2D ColorMap : register(t1, space0);
SamplerState Sampler : register(s0, space0);

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
	//return saturate(saturate(sign(LN)) * pow(saturate(dot(N, normalize(V + L))), LC.a) * LC.rgb * MC); // blinn
}

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	//!< V : ピクセルからカメラへ向かう方向
	const float3 V = normalize(In.TangentViewDirection);

	//!< N	
	const float3 N = NormalMap.Sample(Sampler, In.Texcoord).xyz * 2.0f - 1.0f;

	//!< L : ピクセルからライトへ向かう方向
	const float3 L = normalize(In.TangentLightDirection);

	//!< LN
	const float LN = dot(L, N);

	//!< Color
	const float3 MC = ColorMap.Sample(Sampler, In.Texcoord).rgb;
	const float4 LC = float4(1.0f, 1.0f, 1.0f, 32.0f);

	const float3 Amb = float3(0.25f, 0.25f, 0.25f); //!< * AOMap.Sample(Sampler, In.Texcoord).rgb;
	const float3 Dif = diffuse(MC, LC.rgb, LN);
	const float3 Spc = specular(MC, LC, LN, L, N, V);
	const float Atn = 1.0f; //pow(max(1.0f - 1.0f / LightRange * LightLen, 0.0f), 2.0f);
	const float Spt = 1.0f; //pow(max(1.0f - 1.0f / SpotRange * abs(acos(dot(L, SpotDir))), 1.0f), 0.5f);
	const float3 Rim = 0.0f; //pow(1.0f - max(-V.z, 0.0f), 1.3f) * LC.rgb;
	//const float3 Rim = 0.0f; //pow((1.0f - max(dot(L, n), 0.0f)) * (1.0f - max(-V.z, 0.0f)), 1.3f)* LC.rgb;
	const float3 Hemi = 0.0f; //lerp(GrdCol, SkyCol, dot(n, vec3(0.0f, 1.0f, 0.0f)) * 0.5f + 0.5f);

	Out.Color = float4(Amb + (Dif + Spc) * Atn * Spt + Rim + Hemi, 1.0f);

	//Out.Color = float4(In.Texcoord, 0.0f, 1.0f);
	//Out.Color = float4(NormalMap.Sample(Sampler, In.Texcoord).rgb, 1.0f);

	return Out;
}

