struct IN
{
	float3 Position : POSITION;
};

cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; float3 LocalCameraPosition; };

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Position3D : TEXCOORD2;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float3 CamPos = -float3(View[0][3], View[1][3], View[2][3]);
	const float4x4 PVW = mul(mul(Projection, View), World);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		Out.Position3D = In[i].Position;

		stream.Append(Out);
	}
	stream.RestartStrip();
}
