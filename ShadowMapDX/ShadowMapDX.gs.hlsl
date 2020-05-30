struct IN
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; float4x4 LightProjection; float4x4 LightView; };

struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float4x4 PVW = mul(mul(Projection, View), World);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		Out.Texcoord = In[i].Texcoord;
		Out.Depth = Out.Position.zw;
		stream.Append(Out);
	}
	stream.RestartStrip();
}
