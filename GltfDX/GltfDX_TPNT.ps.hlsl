struct IN
{
	float3 Tangent : TANGENT;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

float4 main(IN In) : SV_TARGET
{
	return float4(normalize(In.Normal) * 0.5f + 0.5f, 1.0f);
	//return float4(normalize(In.Tangent) * 0.5f + 0.5f, 1.0f);
	//return float4(In.Texcoord, 0.0f, 1.0f);
}
