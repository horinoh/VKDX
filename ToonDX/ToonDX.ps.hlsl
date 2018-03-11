struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 ViewDirection : TEXCOORD0;
#if 0
	noperspective float3 TriDistance : TEXCOORD1;
#endif
};

struct OUT
{
	float4 Color : SV_TARGET;
};

float toon(const float shade, const float steps)
{
	return floor(shade * steps) / steps;
	//return ceil(shade * steps) / steps; 
}
float3 diffuse(const float3 MC, const float3 LC, const float LN) 
{ 
	return saturate(toon(saturate(LN), 5.0f) * MC * LC); 
}
float3 specular(const float3 MC, const float4 LC, const float LN, const float3 L, const float3 N, const float3 V)
{
	return saturate(saturate(sign(LN)) * toon(pow(saturate(dot(reflect(-L, N), V)), LC.a), 5.0f) * LC.rgb * MC); // phong
	//return saturate(saturate(sign(LN)) * toon(pow(saturate(dot(N, normalize(V + L))), LC.a), 5.0f) * LC.rgb * MC); // blinn
}

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	
	//!< N
	const float3 N = normalize(In.Normal);

	//!< L
	const float3 LightDirection = float3(0.0f, 1.0f, 0.0f);
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
	const float3 lineColor = float3(0.0f, 0.0f, 0.0f);
	const float lineWidth = 3.0f;
	const float minDist = min(min(In.TriDistance.x, In.TriDistance.y), In.TriDistance.z);
	const float factor = smoothstep(lineWidth - 1.0f, lineWidth + 1.0f, minDist);
	Out.Color.rgb = lerp(lineColor, Out.Color.rgb, factor);
#endif

	//Out.Color = float4(N * 0.5f + 0.5f, 1.0f);

	return Out;
}

