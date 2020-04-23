struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
};

Texture2D NormalMap : register(t0, space0);
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
	
	//!< V
	const float3 V = normalize(In.ViewDirection);

	//!< N
	const float3 n = normalize(In.Normal);
	const float3 t = normalize(In.Tangent - dot(In.Tangent, n) * n);
	const float3 b = cross(n, t);
	const float3x3 tbn = transpose(float3x3(t, b, n));
	const float ParallaxHeight = 0.03f;
	const float2 tc = In.Texcoord + ParallaxHeight * NormalMap.Sample(Sampler, In.Texcoord).a * float2(V.x, -V.y);
	const float3 N = mul(tbn, NormalMap.Sample(Sampler, tc).xyz * 2.0f - 1.0f);

	//!< L
	const float3 LightDirection = float3(0.0f, 1.0f, 0.0f);
	const float3 L = normalize(LightDirection);

	//!< LN
	const float LN = dot(L, N);

	//!< Color
	const float3 MC = float3(0.5f, 0.5f, 0.5f);
	const float4 LC = float4(1.0f, 1.0f, 1.0f, 32.0f);

	const float3 Amb = float3(0.25f, 0.25f, 0.25f);
	const float3 Dif = diffuse(MC, LC.rgb, LN);
	const float3 Spc = specular(MC, LC, LN, L, N, V);
	const float Atn = 1.0f;
	const float Spt = 1.0f;

	Out.Color = float4((Amb + (Dif + Spc) * Atn) * Spt, 1.0f);
	
	//Out.Color = float4(n * 0.5f + 0.5f, 1.0f);
	//Out.Color = float4(t * 0.5f + 0.5f, 1.0f);
	//Out.Color = float4(b * 0.5f + 0.5f, 1.0f); 
	//Out.Color = float4(In.Texcoord, 0.0f, 1.0f);
	//Out.Color = float4(NormalMap.Sample(Sampler, In.Texcoord).xyz, 1.0f);

	return Out;
}

