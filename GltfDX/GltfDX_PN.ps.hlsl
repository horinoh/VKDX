struct IN
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

float4 main(IN In) : SV_TARGET
{
	return float4(normalize(In.Normal) * 0.5f + 0.5f, 1.0f);
}
