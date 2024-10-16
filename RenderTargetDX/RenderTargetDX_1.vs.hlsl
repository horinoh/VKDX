struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

OUT main(uint VertexId : SV_VertexID)
{
	OUT Out;

	Out.Texcoord = float2(VertexId / 2, VertexId % 2);
	Out.Position = float4(Out.Texcoord.x * 2.0f - 1.0f, -(Out.Texcoord.y * 2.0f - 1.0f), 0.0f, 1.0f);
	
#if 0
	const float4x4 TexTransform = float4x4(4.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0, 0.0f, 0.0f, 1.0f);
	Out.Texcoord = mul(TexTransform, float4(Out.Texcoord, 0.0f, 1.0f)).xy;
#endif

	return Out;
}
