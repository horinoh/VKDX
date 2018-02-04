struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

//Texture2D NormalMap : register(t0);
//SamplerState SS : register(s0);

struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	const float3 N = normalize(In.Normal);
	const float3 T = normalize(In.Tangent.xyz - dot(In.Tangent.xyz, N) * N);
	const float3 B = cross(N, T) * In.Tangent.w;
	const float3x3 TBN = transpose(float3x3(T, B, N));

	//const float3 Normal = mul(TBN, NormalMap.Sample(SS, In.Texcoord).xyz * 2.0f - 1.0f);
	//Out.Color = float4(Normal * 0.5f + 0.5f, 1.0f);

	//Out.Color = float4(In.Normal * 0.5f + 0.5f, 1.0f);
	Out.Color = float4(In.Tangent.xyz * 0.5f + 0.5f, 1.0f);
	
	return Out;
}

