struct IN
{
	uint4 Joints : JOINTS0;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float4 Weights : WEIGHTS0;
};

float4 main(IN In) : SV_TARGET
{
	return float4(normalize(In.Normal) * 0.5f + 0.5f, 1.0f);
}
