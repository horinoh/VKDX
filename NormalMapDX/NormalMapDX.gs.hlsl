struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; };

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float3 CamPos = -float3(View[0][3], View[1][3], View[2][3]);
	const float4x4 PVW = mul(mul(Projection, View), World);
	const float4x4 TexTransform = float4x4(2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		Out.Normal = mul((float3x3)World, In[i].Normal);
		Out.Tangent = float4(mul((float3x3)World, In[i].Tangent), 1.0f);
		Out.Texcoord = mul(TexTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
		stream.Append(Out);
	}
	stream.RestartStrip();
}