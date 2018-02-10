struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
};

//Texture2D NormalMap : register(t0);
//SamplerState SS : register(s0);

struct OUT
{
	float4 Color : SV_TARGET;
};

float3 diffuse(const float3 MC, const float3 LC, const float LN) { return saturate(saturate(LN) * MC * LC); }
float3 specular(const float3 MC, const float4 LC, const float LN, const float3 L, const float3 N, const float3 V)
{
	return saturate(saturate(sign(LN)) * pow(saturate(dot(reflect(-L, N), V)), LC.a) * LC.rgb * MC); // phong
	//return saturate(saturate(sign(LN)) * pow(saturate(dot(N, normalize(V + L))), LC.a) * LC.rgb * MC); // blinn
}

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	
	//!< N
	const float3 n = normalize(In.Normal);
	const float3 t = normalize(In.Tangent.xyz - dot(In.Tangent.xyz, n) * n);
	const float3 b = cross(n, t) * In.Tangent.w;
	const float3x3 tbn = transpose(float3x3(t, b, n));
	const float3 N = n;//mul(tbn, NormalMap.Sample(SS, In.Texcoord).xyz * 2.0f - 1.0f); // TODO

	//!< L
	const float3 LightDirection = float3(0, 1, 0); // TODO
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
	
	return Out;
}

