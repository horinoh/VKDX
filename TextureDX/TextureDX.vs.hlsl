//struct IN
//{
//	float3 Position : POSITION;
//};

struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

OUT main(uint VertexId : SV_VertexID)
{
	OUT Out;

	const float2 Position[] = { float2(-1.0f, -1.0f), float2(-1.0f, 1.0f), float2(1.0f, -1.0f), float2(1.0f, 1.0f) };

	Out.Position = float4(Position[VertexId], 0.0f, 1.0f);
	Out.Texcoord = float2(Out.Position.x, -Out.Position.y) * 0.5f + 0.5f;
	
#if 0
	const float4x4 TexTransform = float4x4(4.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0, 0.0f, 0.0f, 1.0f);
	Out.Texcoord = mul(TexTransform, float4(Out.Texcoord, 0.0f, 1.0f)).xy;
#endif

	return Out;
}
